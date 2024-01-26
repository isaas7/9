#include "../include/handler.hpp"
#include <chrono>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

class Messages {
public:
  Messages(const std::string &message)
      : message(message), timestamp(std::chrono::system_clock::now()) {}
  const std::string &getMessage() const { return message; }
  const std::chrono::system_clock::time_point &getTimestamp() const {
    return timestamp;
  }

private:
  std::string message;
  std::chrono::system_clock::time_point timestamp;
};

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req) {
  static std::vector<Messages> messages_;
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
    // spdlog::get("console_logger")->info(res.body());
    return res;
  };
  if (req.method() == http::verb::get) {
    if (req.target() == "/api/messages") {
      try {
        nlohmann::json responseJson;
        for (int i = 0; i < messages_.size(); i++) {
          auto timestamp =
              std::chrono::system_clock::to_time_t(messages_[i].getTimestamp());
          std::tm tm = *std::gmtime(&timestamp);
          nlohmann::json messageJson;
          messageJson["message"] = messages_[i].getMessage();
          std::ostringstream timestampString;
          timestampString << std::put_time(&tm, "%F %T");
          messageJson["timestamp"] = timestampString.str();
          responseJson["messages"].push_back(messageJson);
        }
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = responseJson.dump();
        res.prepare_payload();
        return res;
      } catch (const std::exception &e) {
        return server_error("Error processing request");
      }
    }
  }
  if (req.method() == http::verb::post) {
    if (req.target() == "/api/messages/send")
      try {
        nlohmann::json body = json::parse(req.body());
        std::string msg = body["message"];
        Messages msg_(msg);
        messages_.emplace_back(msg);
        // log message
        std::ostringstream logMessage;
        logMessage << "\n";
        for (int i = 0; i < messages_.size(); i++) {
          auto timestamp =
              std::chrono::system_clock::to_time_t(messages_[i].getTimestamp());
          std::tm tm = *std::gmtime(&timestamp);
          logMessage << i + 1 << ": " << messages_[i].getMessage()
                     << std::put_time(&tm, "%F %T") << "\n";
        }
        spdlog::get("console_logger")->debug(logMessage.str());
        return ok_request("Message sent successfully");
      } catch (const std::exception &e) {
        return server_error("Invalid json body");
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
