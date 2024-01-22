#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <future>
#include <thread>
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

class session : public std::enable_shared_from_this<session> {
  tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
public:
  http::response<http::string_body> res_;

  explicit session(net::io_context &ioc)
      : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}

  void run(char const *host, char const *port, char const *target, int version,
           http::verb method, const std::string &body) {
    req_.version(version);
    req_.method(method);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set(http::field::content_type, "application/json");
    req_.body() = body;
    req_.prepare_payload();

    resolver_.async_resolve(
        host, port,
        beast::bind_front_handler(&session::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec)
      return fail(ec, "resolve");
    stream_.expires_after(std::chrono::seconds(30));
    stream_.async_connect(
        results,
        beast::bind_front_handler(&session::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type) {
    if (ec)
      return fail(ec, "connect");
    stream_.expires_after(std::chrono::seconds(30));
    http::async_write(
        stream_, req_,
        beast::bind_front_handler(&session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");
    http::async_read(
        stream_, buffer_, res_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec)
      return fail(ec, "read");
    std::cout << res_ << std::endl;
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
      return fail(ec, "shutdown");
  }
};

class HttpClientTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(HttpClientTest, AsyncLogin) {
  const char *host = "localhost";
  const char *port = "8080";
  const char *target = "/api/user/login";
  int version = 11;
  net::io_context ioc;
  http::verb method = http::verb::post;
  std::string body = R"({"username": "user", "password": "password"})";
  std::promise<void> promise;
  auto future = promise.get_future();
  auto test_session = std::make_shared<session>(ioc);
  test_session->run(host, port, target, version, method, body);
  std::thread([&ioc, &promise] {
    ioc.run();
    promise.set_value(); // Notify that the operation is completed
  }).detach();
  future.wait();
  ASSERT_EQ(test_session->res_.result(), http::status::ok);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
TEST_F(HttpClientTest, AsyncLoginFail) {
  const char *host = "localhost";
  const char *port = "8080";
  const char *target = "/api/user/login";
  int version = 11;
  net::io_context ioc;
  http::verb method = http::verb::post;
  std::string body = R"({"username": "user", "password": "password"})";
  std::promise<void> promise;
  auto future = promise.get_future();
  auto test_session = std::make_shared<session>(ioc);
  test_session->run(host, port, target, version, method, body);
  std::thread([&ioc, &promise] {
    ioc.run();
    promise.set_value(); // Notify that the operation is completed
  }).detach();
  future.wait();
  ASSERT_EQ(test_session->res_.result(), http::status::internal_server_error);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
TEST_F(HttpClientTest, AsyncRegisterFail) {
  const char *host = "localhost";
  const char *port = "8080";
  const char *target = "/api/user/register";
  int version = 11;
  net::io_context ioc;
  http::verb method = http::verb::post;
  std::string body = R"({"username": "user", "password": "password"})";
  std::promise<void> promise;
  auto future = promise.get_future();
  auto test_session = std::make_shared<session>(ioc);
  test_session->run(host, port, target, version, method, body);
  std::thread([&ioc, &promise] {
    ioc.run();
    promise.set_value(); // Notify that the operation is completed
  }).detach();
  future.wait();
  ASSERT_EQ(test_session->res_.result(), http::status::internal_server_error);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
