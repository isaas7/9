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
size_t count_char(const std::string &target, char ch);

template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
               PgConnectionPool &pg_pool,
               std::unordered_map<std::string, SessionData> &session_storage) {
  size_t num_slashes = count_char(req.target(), '/');
  size_t num_questions = count_char(req.target(), '?');
  size_t num_equals = count_char(req.target(), '=');
  size_t num_ands = count_char(req.target(), '&');
  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
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
    return res;
  };
  auto const login_api = [&req, &pg_pool,
                          &session_storage](const std::vector<std::string>
                                                &username_password) {
    if (username_password.size() < 2) {
      return http::response<http::string_body>{http::status::bad_request,
                                               req.version()};
    }
    const std::string &username = username_password[0];
    const std::string &password = username_password[1];
    std::string query = "SELECT * FROM mock_data WHERE username = '" +
                        username + "' AND password = '" + password + "';";
    pqxx::result result = pg_pool.executeSelect(query);
    if (!result.empty()) {
      http::response<http::string_body> res{http::status::ok, req.version()};
      std::string session_id =
          "session_" +
          std::to_string(
              std::chrono::system_clock::now().time_since_epoch().count());
      std::cout << "login_api session id: " << session_id << std::endl
                << "login_api session_storage size: " << session_storage.size()
                << std::endl;
      auto existing_session_it =
          std::find_if(session_storage.begin(), session_storage.end(),
                       [&username](const auto &session_pair) {
                         return session_pair.second.username == username;
                       });
      if (existing_session_it != session_storage.end())
        session_storage.erase(existing_session_it);
      SessionData session_data;
      session_data.username = username;
      session_data.expiration_time =
          std::chrono::steady_clock::now() + std::chrono::minutes(30);
      session_storage[session_id] = session_data;
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/plain");
      res.keep_alive(req.keep_alive());
      res.body() = "Authentication successful";
      res.set(http::field::authorization, "Bearer " + session_id);
      res.prepare_payload();
      return res;
    }
    http::response<http::string_body> res{http::status::unauthorized,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = "Authentication failed";
    res.prepare_payload();
    return res;
  };
  auto const unauthenticated_route =
      [&req, &pg_pool, &session_storage](beast::string_view target) {
        http::response<http::string_body> res{http::status::unauthorized,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.prepare_payload();
        return res;
      };
  auto const authenticated_route = [&req, &pg_pool, &session_storage](
                                       beast::string_view target) {
    std::cout << "request to authenticated_products made " << std::endl;
    std::string authorization_header = req[http::field::authorization];
    std::string bearer_prefix = "Bearer ";
    if (authorization_header.size() > bearer_prefix.size() &&
        std::equal(bearer_prefix.begin(), bearer_prefix.end(),
                   authorization_header.begin())) {
      std::string token = authorization_header.substr(bearer_prefix.size());
      std::cout << "authenticated_products token: " << token << std::endl;
      if (!validate_token(token, session_storage)) {
        std::string query = "SELECT * FROM mock_data_products;";
        pqxx::result result = pg_pool.executeSelect(query);
        nlohmann::json json_response;
        for (const auto &row : result) {
          nlohmann::json row_json;
          row_json["id"] = row["id"].as<int>();
          row_json["name"] = row["name"].as<std::string>();
          row_json["price"] = row["price"].as<double>();
          row_json["quantity"] = row["quantity"].as<std::string>();
          json_response.push_back(row_json);
        }
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = json_response.dump();
        res.prepare_payload();
        std::cout << "token matches" << std::endl;
        return res;
      }
    }
    http::response<http::string_body> res{http::status::unauthorized,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.prepare_payload();
    return res;
  };
  if (req.method() == http::verb::post) {
    if (req.target().starts_with("/api?=login")) {
      std::string target = std::string(req.target());
      size_t username_pos = target.find("&username=");
      size_t password_pos = target.find("&password=");
      if (username_pos != std::string::npos &&
          password_pos != std::string::npos) {
        std::string username = target.substr(
            username_pos + 10, password_pos - (username_pos + 10));
        std::string password = target.substr(password_pos + 10);
        std::cout << "Login attempt with username: " << username
                  << ", password: " << password << std::endl;
        std::vector<std::string> userpass;
        userpass.emplace_back(username);
        userpass.emplace_back(password);
        return login_api(userpass);
      }
    }
  }
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return bad_request("Unknown HTTP-method");
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");
  if (req.target().back() == '/' && req.target().size() == 1)
    return unauthenticated_route(req.target());
  if (num_slashes == 1 && num_questions == 1) {
    if (num_equals == 0)
      return bad_request("Illegal request-target");
    if (req.target() == "/api?=authenticated_products" &&
        req.method() == http::verb::get)
      return authenticated_route(req.target());
    else
      return bad_request("Illegal request-target");
  } else {
    return unauthenticated_route(req.target());
  }
}
#endif
