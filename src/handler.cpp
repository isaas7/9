// handler.cpp
#include "../include/handler.hpp"
#include <map>
#include <string>

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               std::shared_ptr<ChatService> chatService) {
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
  auto const get_messages_service = [&req, &statusMap, &chatService,
                                     &http_response](int roomNumber) {
    auto allRooms = chatService->getMsgService();
    if (roomNumber >= 0 && roomNumber < allRooms.size()) {
      auto &specifiedRoom = allRooms[roomNumber];
      json response_json;
      for (const auto &message : specifiedRoom.getMessages()) {
        std::string msgstr = message.getMsgString();
        spdlog::get("console_logger")
            ->debug("Message in room {}: {}", roomNumber, msgstr);
        response_json["messages"].push_back({{"message", msgstr}});
      }
      std::string response_str = response_json.dump();
      auto status = statusMap["ok_request"];
      http::response<http::string_body> res{status, req.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "application/json");
      res.keep_alive(req.keep_alive());
      res.body() = response_str;
      res.prepare_payload();
      spdlog::get("console_logger")
          ->debug("ok_request occurred with messages: {}", response_str);
      spdlog::get("console_logger")
          ->debug("ChatService rooms size: {}",
                  chatService->getMsgService().size());
      return res;
    } else {
      return http_response("not_found", "Specified room not found");
    }
  };
  auto const send_messages_service = [&req, &statusMap, &chatService,
                                      &http_response](std::string msg,
                                                      int roomnum) {
    spdlog::get("console_logger")->debug("Entering send_messages_service");
    auto &allRooms = chatService->getMsgService();
    if (allRooms.size() == 0)
      chatService->addRoom();
    if (roomnum < 0 || roomnum >= allRooms.size())
      return http_response("not_found", "Specified room not found");
    spdlog::get("console_logger")
        ->debug("Existing rooms found. Adding message to room {}.", roomnum);
    auto &specifiedRoom = allRooms[roomnum];
    specifiedRoom.addMessage(msg);
    json response_json = {{"message", "success"}};
    std::string response_str = response_json.dump();
    auto status = statusMap["ok_request"];
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = response_str;
    res.prepare_payload();
    spdlog::get("console_logger")
        ->debug("Exiting send_messages_service. ChatService rooms size: {}",
                allRooms.size()); // Use reference to allRooms
    return res;
  };

  if (req.method() == http::verb::post) {
    if (req.target() == "/api/messages") {
      try {
        json body = json::parse(req.body());
        if (body.find("room") == body.end()) {
          return http_response("bad_request",
                               "Missing 'room' key in JSON body");
        }
        int roomNumber = body["room"].get<int>();
        return get_messages_service(roomNumber);
      } catch (const std::exception &e) {
        return http_response("server_error", "Invalid json body");
      }
    }
    if (req.target() == "/api/messages/send") {
      try {
        json body = json::parse(req.body());
        if (body.find("room") == body.end()) {
          return http_response("bad_request",
                               "Missing 'room' key in JSON body");
        }
        std::string msg = body["message"];
        int roomNumber = body["room"].get<int>();
        return send_messages_service(msg, roomNumber);
      } catch (const std::exception &e) {
        return http_response("server_error", "Invalid json body");
      }
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
    std::shared_ptr<ChatService> chatService);
