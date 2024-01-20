#include "../include/pgconnectionpool.hpp"
#include <boost/asio/query.hpp>
#include <iostream>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
PgConnectionPool::PgConnectionPool(const std::string &conn_str,
                                   size_t pool_size, const dbSchema_ &schema)
    : conn_str_(conn_str), pool_size_(pool_size), schema_(schema) {
  auto console_logger = spdlog::stdout_color_mt("console_logger");
  // auto file_logger = spdlog::basic_logger_mt("file_logger", "logfile.txt");
  console_logger->set_level(spdlog::level::debug);
  spdlog::set_default_logger(console_logger);
  spdlog::flush_every(std::chrono::seconds(3));
  for (size_t i = 0; i < pool_size_; ++i) {
    auto connection = std::make_shared<pqxx::connection>(conn_str_);
    connections_.emplace_back(connection);
  }
}

std::shared_ptr<pqxx::connection> PgConnectionPool::get_connection() {
  spdlog::get("console_logger")->debug("PgConnectionPool::get_connection");
  return connections_.front();
}

void PgConnectionPool::selectQuery() {}

bool PgConnectionPool::selectQuery(const std::string &table,
                                   const std::string &username) {
  spdlog::get("console_logger")
      ->debug("bool PgConnectionPool::selectQuery (username validation)");
  const std::string query = "SELECT * FROM " + table + " WHERE " +
                            schema_.db_schema.at(table)[0] + " = $1";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec_params(query, username);
    transaction.commit();
    return !result.empty();
  } catch (const std::exception &e) {
    spdlog::error("Exception caught in selectQuery: {}", e.what());
    throw;
  }
}

bool PgConnectionPool::selectQuery(const std::string &table,
                                   const std::string &username,
                                   const std::string &password) {
  spdlog::get("console_logger")
      ->debug(
          "bool PgConnectionPool::selectQuery (username, password validation)");
  const std::string query = "SELECT * FROM " + table + " WHERE " +
                            schema_.db_schema.at(table)[0] + " = $1 AND " +
                            schema_.db_schema.at(table)[1] + " = $2";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec_params(query, username, password);
    transaction.commit();
    return !result.empty();
  } catch (const std::exception &e) {
    spdlog::error("Exception caught in selectQuery: {}", e.what());
    throw;
  }
}

bool PgConnectionPool::insertQuery(const std::string &table,
                                   const std::string &username,
                                   const std::string &password) {
  spdlog::get("console_logger")
      ->debug("bool PgConnectionPool::insertQuery (username, password "
              "registration)");
  if (selectQuery("example_table", username)) {
    spdlog::error("Exception caught in insertQuery : Username already exists");
    return false;
  }
  const std::string query =
      "INSERT INTO " + table + "(" + schema_.db_schema.at(table)[0] + ", " +
      schema_.db_schema.at(table)[1] + ") VALUES ($1, $2)";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    transaction.exec_params(query, username, password);
    transaction.commit();
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Exception caught in insertQuery: {}", e.what());
    return false;
  }
}
