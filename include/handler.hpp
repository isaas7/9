#ifndef HANDLER_HPP
#define HANDLER_HPP
#include <nlohmann/json.hpp>
#include "beast.hpp"
#include "util.hpp"
using json = nlohmann::json;

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req);
#endif
