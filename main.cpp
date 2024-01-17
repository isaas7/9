#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class PgConnectionPool {
public:
  PgConnectionPool(const std::string &conn_str, size_t pool_size)
      : conn_str_(conn_str), pool_size_(pool_size) {
    for (size_t i = 0; i < pool_size_; ++i) {
      auto connection = std::make_shared<pqxx::connection>(conn_str_);
      connections_.emplace_back(connection);
    }
  }

  std::shared_ptr<pqxx::connection> get_connection() {
    std::cout << "get_connection()" << std::endl;
    return connections_.front();
  }

private:
  std::string conn_str_;
  size_t pool_size_;
  std::vector<std::shared_ptr<pqxx::connection>> connections_;
};

class queryAbstraction {
public:
  queryAbstraction(PgConnectionPool &pg_pool) : pg_pool_(pg_pool) {}

  pqxx::result executeSelect(const std::string &query) {
    try {
      auto connection = pg_pool_.get_connection();
      pqxx::work transaction(*connection);
      pqxx::result result = transaction.exec(query);
      transaction.commit();
      return result;
    } catch (const std::exception &e) {
      std::cerr << "Exception caught in executeSelect: " << e.what()
                << std::endl;
      throw;
    }
  }

private:
  PgConnectionPool &pg_pool_;
};

class SessionData {
public:
  std::string username;
  std::chrono::steady_clock::time_point expiration_time;
};

bool validate_token(
    const std::string &token,
    const std::unordered_map<std::string, SessionData> &session_storage) {
  auto session_it = session_storage.find(token);
  return session_it != session_storage.end() &&
         session_it->second.expiration_time > std::chrono::steady_clock::now();
}

size_t count_char(const std::string &target, char ch) {
  return std::count(target.begin(), target.end(), ch);
}

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               PgConnectionPool &pg_pool,
               std::unordered_map<std::string, SessionData> &session_storage,
               queryAbstraction &query_abstraction) {
  size_t num_slashes = count_char(req.target(), '/');
  size_t num_questions = count_char(req.target(), '?');
  size_t num_equals = count_char(req.target(), '=');
  size_t num_ands = count_char(req.target(), '&');

  auto const login_api = [&req, &pg_pool, &session_storage,
                          &query_abstraction](const std::vector<std::string>
                                                  &username_password) {
    if (username_password.size() < 2) {
      return http::response<http::string_body>{http::status::bad_request,
                                               req.version()};
    }
    const std::string &username = username_password[0];
    const std::string &password = username_password[1];
    std::string query = "SELECT * FROM mock_data WHERE username = '" +
                        username + "' AND password = '" + password + "';";
    pqxx::result result = query_abstraction.executeSelect(query);
    if (!result.empty()) {
      http::response<http::string_body> res{http::status::ok, req.version()};
      std::string session_id =
          "session_" +
          std::to_string(
              std::chrono::system_clock::now().time_since_epoch().count());
      std::cout << "login_api session id: " << session_id << std::endl
                << "login_api session_storage size: " << session_storage.size()
                << std::endl;
      SessionData session_data;
      session_data.username = username;
      session_data.expiration_time =
          std::chrono::steady_clock::now() + std::chrono::minutes(30);
      session_storage[session_id] = session_data;
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/plain");
      res.keep_alive(req.keep_alive());
      res.body() = "Authentication successful";
      res.set(http::field::authorization, "Bearer " + session_id);
      res.prepare_payload();
      return res;
    }
    http::response<http::string_body> res{http::status::unauthorized,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = "Authentication failed";
    res.prepare_payload();
    return res;
  };
  auto const unauthenticated_route = [&req, &pg_pool, &session_storage,
                                      &query_abstraction](
                                         beast::string_view target) {
    http::response<http::string_body> res{http::status::unknown, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  };
  auto const authenticated_route = [&req, &pg_pool, &session_storage,
                                    &query_abstraction](
                                       beast::string_view target) {
    std::cout << "request to authenticated_products made " << std::endl;
    std::string authorization_header = req[http::field::authorization];
    std::string bearer_prefix = "Bearer ";
    if (authorization_header.size() > bearer_prefix.size() &&
        std::equal(bearer_prefix.begin(), bearer_prefix.end(),
                   authorization_header.begin())) {
      std::string token = authorization_header.substr(bearer_prefix.size());
      std::cout << "authenticated_products token: " << token << std::endl;
      if (validate_token(token, session_storage)) {
        std::string query = "SELECT * FROM mock_data_products;";
        pqxx::result result = query_abstraction.executeSelect(query);
        nlohmann::json json_response;
        for (const auto &row : result) {
          nlohmann::json row_json;
          row_json["id"] = row["id"].as<int>();
          row_json["name"] = row["name"].as<std::string>();
          row_json["price"] = row["price"].as<double>();
          row_json["quantity"] = row["quantity"].as<std::string>();
          json_response.push_back(row_json);
        }
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = json_response.dump();
        res.prepare_payload();
        std::cout << "token matches" << std::endl;
        return res;
      }
    }
    http::response<http::string_body> res{http::status::unauthorized,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  };

  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  auto const not_found = [&req](beast::string_view target) {
    http::response<http::string_body> res{http::status::not_found,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  };

  auto const server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  if (req.method() == http::verb::post) {
    if (req.target().starts_with("/api?=login")) {
      std::string target = std::string(req.target());

      size_t username_pos = target.find("&username=");
      size_t password_pos = target.find("&password=");

      if (username_pos != std::string::npos &&
          password_pos != std::string::npos) {
        std::string username = target.substr(
            username_pos + 10, password_pos - (username_pos + 10));
        std::string password = target.substr(password_pos + 10);
        std::cout << "Login attempt with username: " << username
                  << ", password: " << password << std::endl;
        std::vector<std::string> userpass;
        userpass.emplace_back(username);
        userpass.emplace_back(password);
        return login_api(userpass);
      }
    }
  }
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return bad_request("Unknown HTTP-method");

  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");

  if (req.target().back() == '/' && req.target().size() == 1)
    return unauthenticated_route(req.target());
  if (num_slashes == 1 && num_questions == 1) {
    if (num_equals == 0) {
      return bad_request("Illegal request-target");
    }
    if (req.target() == "/api?=authenticated_products" &&
        req.method() == http::verb::get) {
      return authenticated_route(req.target());
    } else {
      return bad_request("Illegal request-target");
    }
  } else {
    return unauthenticated_route(req.target());
  }
}

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

class session : public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  PgConnectionPool &pg_pool_;
  std::unordered_map<std::string, SessionData> session_storage_;
  queryAbstraction &query_abstraction_;

public:
  session(tcp::socket &&socket, PgConnectionPool &pg_pool,
          std::unordered_map<std::string, SessionData> &session_storage,
          queryAbstraction &query_abstraction)
      : stream_(std::move(socket)), pg_pool_(pg_pool),
        session_storage_(session_storage),
        query_abstraction_(query_abstraction) {}

  void run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&session::do_read, shared_from_this()));
  }

  void do_read() {
    req_ = {};

    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
      return do_close();

    if (ec)
      return fail(ec, "read");

    send_response(handle_request(std::move(req_), pg_pool_, session_storage_,
                                 query_abstraction_));
  }

  void send_response(http::message_generator &&msg) {
    bool keep_alive = msg.keep_alive();

    beast::async_write(stream_, std::move(msg),
                       beast::bind_front_handler(
                           &session::on_write, shared_from_this(), keep_alive));
  }

  void on_write(bool keep_alive, beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    if (!keep_alive) {
      return do_close();
    }

    do_read();
  }

  void do_close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  }
};

class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  PgConnectionPool &pg_pool_;
  std::unordered_map<std::string, SessionData> session_storage_;
  queryAbstraction &query_abstraction_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           PgConnectionPool &pg_pool,
           std::unordered_map<std::string, SessionData> &session_storage,
           queryAbstraction &query_abstraction)
      : ioc_(ioc), acceptor_(net::make_strand(ioc)), pg_pool_(pg_pool),
        session_storage_(session_storage),
        query_abstraction_(query_abstraction) {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  void run() { do_accept(); }

private:
  void do_accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
      return;
    } else {
      std::make_shared<session>(std::move(socket), pg_pool_, session_storage_,
                                query_abstraction_)
          ->run();
    }

    do_accept();
  }
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: http-server-async <address> <port> <threads>\n"
              << "Example:\n"
              << "    http-server-async 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }
  PgConnectionPool pg_pool("dbname=database_2024 user=user "
                           "password=password hostaddr=172.19.0.2 port=5432",
                           5);
  std::unordered_map<std::string, SessionData> session_storage;
  auto const address = net::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const threads = std::max<int>(1, std::atoi(argv[3]));
  queryAbstraction query_abstraction(pg_pool);
  net::io_context ioc{threads};

  std::make_shared<listener>(ioc, tcp::endpoint{address, port}, pg_pool,
                             session_storage, query_abstraction)
      ->run();

  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();

  return EXIT_SUCCESS;
}
