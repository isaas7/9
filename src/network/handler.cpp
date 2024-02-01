#include "../../include/network/handler.hpp"
#include <map>
#include <string>

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               std::shared_ptr<Application> app) {
  std::map<std::string, http::status> statusMap = {
      {"bad", http::status::bad_request},
      {"not_found", http::status::not_found},
      {"server_error", http::status::internal_server_error},
      {"ok_request", http::status::ok}};

  auto const http_response = [&req, &statusMap](std::string which,
                                                beast::string_view str) {
    auto status = statusMap[which];
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(str);
    res.prepare_payload();
    spdlog::get("console_logger")->debug("{} occurred: {}", which, str);
    return res;
  };
  if (req.method() == http::verb::post && req.target() == "/game") {
    try {
      json requestJson = json::parse(req.body());
      std::string username1 = requestJson["user1"];
      std::string username2 = requestJson["user2"];

      auto userService = app->getUserService();
      auto gameService = app->getGameService();

      if (userService->userExists(username1) &&
          userService->userExists(username2)) {
        auto user1 = userService->getUser(username1);
        auto user2 = userService->getUser(username2);

        auto winner = gameService->playGame(user1, user2);

        return http_response("ok_request", winner.getUsername());
      } else {
        return http_response("bad_request", "Invalid user(s) for the game");
      }
    } catch (const json::exception &e) {
      return http_response("bad_request", "Invalid JSON format");
    }
  }
  if (req.method() == http::verb::get && req.target() == "/messages") {
    auto messageService = app->getMessageService();
    auto messages = messageService->getMessages();
    json responseJson;

    for (const auto &message : messages) {
      responseJson["messages"].push_back(message.getMessage());
    }

    return http_response("ok_request", responseJson.dump());
  }

  if (req.method() == http::verb::post && req.target() == "/messages/send") {
    try {
      json requestJson = json::parse(req.body());
      std::string message = requestJson["message"];
      auto messageService = app->getMessageService();
      messageService->addMessage(message);
      return http_response("ok_request", "Message added successfully");
    } catch (const json::exception &e) {
      return http_response("bad_request", "Invalid JSON format");
    }
  }
  if (req.method() == http::verb::post &&
      req.target() == "/messages/private/send") {
    try {
      json requestJson = json::parse(req.body());
      std::string sender = requestJson["sender"];
      std::string receiver = requestJson["receiver"];
      std::string message = requestJson["message"];

      auto userService = app->getUserService();
      auto privateMessageService = app->getPrivateMessageService();

      if (userService->userExists(sender) &&
          userService->userExists(receiver)) {
        auto privateMessageService = app->getPrivateMessageService();
        privateMessageService->addMessage(sender, receiver, message);
        return http_response("ok_request", "Private message sent successfully");
      } else {
        return http_response("bad_request", "Invalid sender or receiver");
      }
    } catch (const json::exception &e) {
      return http_response("bad_request", "Invalid JSON format");
    }
  }
  if (req.method() == http::verb::post &&
      req.target() == "/messages/private/get") {
    try {
      json requestJson = json::parse(req.body());
      std::string sender = requestJson["sender"];
      std::string receiver = requestJson["receiver"];

      auto userService = app->getUserService();
      auto privateMessageService = app->getPrivateMessageService();

      if (userService->userExists(sender) &&
          userService->userExists(receiver)) {
        // Use the same instance of User for comparison
        auto senderUser = userService->getUser(sender);
        auto receiverUser = userService->getUser(receiver);

        auto messages =
            privateMessageService->getMessages(senderUser, receiverUser);

        json responseJson;
        for (const auto &message : messages) {
          responseJson["messages"].push_back(message.getMessage());
        }

        return http_response("ok_request", responseJson.dump());
      } else {
        return http_response("bad_request", "Invalid sender or receiver");
      }
    } catch (const json::exception &e) {
      return http_response("bad_request", "Invalid JSON format");
    }
  }
  if (req.method() == http::verb::post && req.target() == "/user/add") {
    try {
      json requestJson = json::parse(req.body());
      std::string username = requestJson["username"];

      auto userService = app->getUserService();
      userService->addUser(username);

      return http_response("ok_request", "User added successfully");
    } catch (const json::exception &e) {
      return http_response("bad_request", "Invalid JSON format");
    }
  }

  if (req.target().back() == '/' && req.method() == http::verb::get)
    return http_response("ok_request", req.target());

  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return http_response("bad_request", "Unknown HTTP-method");

  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return http_response("bad_request", "Illegal request-target");

  return http_response("not_found", req.target());
}

template http::message_generator handle_request(
    http::request<http::string_body, http::basic_fields<std::allocator<char>>>
        &&req,
    std::shared_ptr<Application> app);
