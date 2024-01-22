#include "../include/handler.hpp"
#include <exception>

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               PgConnectionPool &pg_pool) {
  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    spdlog::get("console_logger")->warn("Bad request occurred: {}", why);
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
    spdlog::get("console_logger")
        ->warn("The resource '{}' was not found", target);
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
    spdlog::get("console_logger")->error("An error ocurred: '{}'", what);
    return res;
  };
  auto const ok_request = [&req](beast::string_view target) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource returns \"" + std::string(target) + "\"";
    res.prepare_payload();
    spdlog::get("console_logger")->info(res.body());
    return res;
  };

  if (req.method() == http::verb::post) {
    if (req.target() == "/api/user/login") {
      try {
        json body = json::parse(req.body());
        std::string username = body["username"];
        std::string password = body["password"];
        spdlog::get("console_logger")->info("/api/user/login request");
        if (pg_pool.selectQuery("user_table", username, password)) {
          const auto &session_storage = pg_pool.getSessions();
          auto sessionIt = session_storage.find(username);
          if (sessionIt != session_storage.end()) {
            std::string sessionToken = sessionIt->second.sessionToken;
            std::string bearerToken = "Bearer " + sessionToken;
            http::response<http::string_body> res{http::status::ok,
                                                  req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.set(http::field::authorization, bearerToken);
            res.keep_alive(req.keep_alive());
            res.body() = "Login successful for user: " + username;
            res.prepare_payload();
            spdlog::get("console_logger")->info(res.body());
            return res;
          } else {
            return server_error("Session information not found");
          }
        } else {
          return server_error("Authentication failure");
        }
      } catch (const std::exception &e) {
        return bad_request("Invalid request body");
      }
    }
    if (req.target() == "/api/user/register")
      try {
        json body = json::parse(req.body());
        std::string username = body["username"];
        std::string password = body["password"];
        spdlog::get("console_logger")->info("/api/user/register request");
        if (pg_pool.insertQuery("user_table", username, password))
          return ok_request("Registration successful for user: " + username);
        else
          return server_error("Registration failure for user: " + username);
      } catch (const std::exception &e) {
        return bad_request("Invalid request body");
      }
    if (req.target() == "/api/user/logout") {
      try {
        json body = json::parse(req.body());
        std::string username = body["username"];
        std::string sessionToken =
            body["session"]; // Get sessionToken from request body
        const auto &session_storage = pg_pool.getSessions();
        auto it = session_storage.find(username);
        if (it != session_storage.end() &&
            it->second.sessionToken == sessionToken &&
            it->second.expiration_time > std::chrono::steady_clock::now()) {
          pg_pool.deleteSession(username);
          return ok_request("Logout successful for user: " + username);
        } else {
          return server_error("Invalid session or session expired");
        }
      } catch (const std::exception &e) {
        return bad_request("Invalid request body");
      }
    }
  }
  if (req.method() == http::verb::get) {
    if (req.target() == "/api/users") {
      try {
        pqxx::result usersResult = pg_pool.selectQuery("user_table");
        json usersJson;
        for (const auto &row : usersResult) {
          json userJson;
          for (const auto &column : row) {
            userJson[column.name()] = column.c_str();
          }
          usersJson.push_back(userJson);
        }
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = usersJson.dump();
        res.prepare_payload();
        spdlog::get("console_logger")->info("User list retrieved");
        return res;
      } catch (const std::exception &e) {
        return server_error(e.what());
      }
    }
  }
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return bad_request("Unknown HTTP-method");
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");
  if (req.target().back() == '/')
    return ok_request(req.target());
  return server_error("server error");
}
template http::message_generator handle_request(
    http::request<http::string_body, http::basic_fields<std::allocator<char>>>
        &&req,
    PgConnectionPool &pg_pool);
