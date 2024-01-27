#include "../include/handler.hpp"
#include <string>
template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               ChatSystem &chatSystem) {
  //
  //
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
  auto const ok_request = [&req](std::string str) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = str;
    res.prepare_payload();
    // spdlog::get("console_logger")->info(res.body());
    return res;
  };
  //
  //
  if (req.method() == http::verb::get) {
  }
  if (req.method() == http::verb::post) {
    if (req.target() == "/api/messages") {
      try {
        nlohmann::json body = json::parse(req.body());
        std::string room = body["room"];
        int intRoom = std::stoi(room);

        // Get messages for the specified room
        const std::vector<Messages> &roomMessages =
            chatSystem.getRooms()[intRoom].getMessages();

        // Convert messages to JSON format
        nlohmann::json responseJson;
        for (const auto &message : roomMessages) {
          nlohmann::json messageJson;
          messageJson["message"] = message.getMessage();
          messageJson["timestamp"] =
              message.getTimestamp().time_since_epoch().count();
          responseJson.push_back(messageJson);
        }

        // Return the JSON response
        return ok_request(responseJson.dump());
      } catch (const std::exception &e) {
        return bad_request("Invalid JSON body");
      }
    }
    if (req.target() == "/api/messages/send")
      try {
        nlohmann::json body = json::parse(req.body());
        std::string msg = body["message"];
        std::string room = body["room"];
        int intRoom = std::stoi(room);
        chatSystem.sendMessageToRoom(intRoom, msg);
        return ok_request("send message success");
      } catch (const std::exception &e) {
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
        &&req,
    ChatSystem &chatSystem);
