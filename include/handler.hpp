#ifndef HANDLER_HPP
#define HANDLER_HPP
#include "beast.hpp"
#include "net.hpp"
#include "pgconnectionpool.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;

bool validate_token(
    const std::string &token,
    const std::unordered_map<std::string, SessionData> &session_storage);

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               PgConnectionPool &pg_pool);
#endif
