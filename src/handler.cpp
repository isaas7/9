#include "../include/handler.hpp"
#include <chrono>
#include <exception>
#include <spdlog/spdlog.h>
#include <string>

std::string generate_token() {
  auto now = std::chrono::system_clock::now();
  auto expirationTime = now + std::chrono::seconds(60);
  auto expirationDuration =
      std::chrono::duration_cast<std::chrono::seconds>(expirationTime - now);
  std::time_t expirationTimestamp =
      std::chrono::system_clock::to_time_t(now + expirationDuration);
  std::string sessionToken = "Token_" + std::to_string(expirationTimestamp);
  spdlog::get("console_logger")->info("sessionToken: {}", sessionToken);
  return sessionToken;
}
bool validate_token(std::string token) {
  std::time_t expirationTimestamp =
      std::stoi(token.substr(6)); // Assuming the timestamp is part of the token
  auto now = std::chrono::system_clock::now();
  if (now > std::chrono::system_clock::from_time_t(expirationTimestamp))
    return false;
  else
    return true;
}

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req) {
  static std::vector<std::string> messages_;
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
  auto const messages_request = [&req, &ok_request,
                                 &bad_request](beast::string_view target,
                                               std::string sessionToken) {
    spdlog::get("console_logger")->info("messages_request");
    nlohmann::json messagesJson;
    for (const auto &message : messages_) {
      messagesJson.push_back(message);
    }
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = messagesJson.dump();
    res.prepare_payload();
    return res;
  };
  auto const send_message = [&req, &ok_request,
                             &bad_request](beast::string_view target,
                                           std::string sessionToken) {
    spdlog::get("console_logger")->info("send_message");
    messages_.emplace_back(sessionToken);
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = "success";
    res.prepare_payload();
    return res;
  };

  auto const polling_request = [&req, &ok_request, &bad_request,
                                &messages_request](beast::string_view target,
                                                   std::string sessionToken) {
    spdlog::get("console_logger")->info("polling_request");
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    while (validate_token(sessionToken)) {
      return messages_request("valid token", sessionToken);
    }
    return bad_request("token_expired");
  };
  auto const token_request = [&req, &ok_request](beast::string_view target,
                                                 std::string token) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = token;
    res.prepare_payload();
    spdlog::get("console_logger")->info(res.body());
    return res;
  };
  if (req.method() == http::verb::post) {
    if (req.target() == "/api/heartbeat")
      try {
        nlohmann::json body = nlohmann::json::parse(req.body());
        std::string token = body["token"];
        return polling_request(req.target(), token);
      } catch (const std::exception &e) {
        spdlog::get("console_logger")
            ->error("Failed to parse JSON body: {}", e.what());
        return bad_request("Invalid JSON body");
      }
    if (req.target() == "/api/messages/send")
      try {
        nlohmann::json body = nlohmann::json::parse(req.body());
        std::string token = body["token"];
        std::string message = body["message"];
        return send_message(req.target(), message);
      } catch (const std::exception &e) {
        spdlog::get("console_logger")
            ->error("Failed to parse JSON body: {}", e.what());
        return bad_request("Invalid JSON body");
      }
    if (req.target() == "/api/messages")
      try {
        nlohmann::json body = nlohmann::json::parse(req.body());
        std::string token = body["token"];
        if (validate_token(token))
          return messages_request(req.target(), token);
      } catch (const std::exception &e) {
        spdlog::get("console_logger")
            ->error("Failed to parse JSON body: {}", e.what());
        return bad_request("Invalid JSON body");
      }
  }
  if (req.method() == http::verb::get) {
    if (req.target() == "/api/token")
      try {
        std::string token = generate_token();
        return token_request(req.target(), token);
      } catch (const std::exception &e) {
        spdlog::get("console_logger")
            ->error("Failed to parse JSON body: {}", e.what());
        return bad_request("Invalid JSON body");
      }
  }
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return bad_request("Unknown HTTP-method");
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");
  if (req.target().back() == '/')
    return ok_request(req.target());
  else
    return not_found(req.target());
}
template http::message_generator handle_request(
    http::request<http::string_body, http::basic_fields<std::allocator<char>>>
        &&req);
