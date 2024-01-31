#include "../include/handler.hpp"
#include <map>
#include <string>
template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req) {
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
        &&req);
