// handler.cpp
#include "../include/handler.hpp"
#include <map>
#include <string>

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               std::shared_ptr<MessageService> messageService) {
  std::map<std::string, http::status> statusMap = {
      {"bad", http::status::bad_request},
      {"not_found", http::status::not_found},
      {"server_error", http::status::internal_server_error},
      {"ok_request", http::status::ok}};
  auto const http_response = [&req, &statusMap, &messageService](
                                 std::string which, beast::string_view str) {
    spdlog::get("console_logger")
        ->debug("MessageService messages size: {}",
                messageService->getMessages().size());
    http::status status = statusMap[which];
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(str);
    res.prepare_payload();
    spdlog::get("console_logger")->debug("{} occurred: {}", which, str);
    return res;
  };
  auto const messages_service = [&req, &statusMap,
                                 &messageService](std::string msg) {
    messageService->addMessage(msg); // Assuming you want to add the received
                                     // message to the MessageService

    // Construct the JSON response
    json response_json = {{"message", "success"}};
    std::string response_str = response_json.dump();

    http::status status = statusMap["ok_request"];
    http::response<http::string_body> res{status, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type,
            "application/json"); // Set content type to JSON
    res.keep_alive(req.keep_alive());
    res.body() = response_str;
    res.prepare_payload();
    spdlog::get("console_logger")
        ->debug("ok_request occurred: {}", response_str);
    spdlog::get("console_logger")
        ->debug("MessageService messages size: {}",
                messageService->getMessages().size());
    return res;
  };
  if (req.method() == http::verb::post) {
    if (req.target() == "/api/messages/send") {
      try {
        json body = json::parse(req.body());
        std::string msg = body["message"];
        return messages_service(msg);
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
    std::shared_ptr<MessageService> messageService);
