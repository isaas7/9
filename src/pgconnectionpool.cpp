#include "../include/pgconnectionpool.hpp"
#include <boost/asio/query.hpp>
#include <iostream>
PgConnectionPool::PgConnectionPool(const std::string &conn_str,
                                   size_t pool_size)
    : conn_str_(conn_str), pool_size_(pool_size) {
  for (size_t i = 0; i < pool_size_; ++i) {
    auto connection = std::make_shared<pqxx::connection>(conn_str_);
    connections_.emplace_back(connection);
  }
}
std::shared_ptr<pqxx::connection> PgConnectionPool::get_connection() {
  std::cout << "PgConnectionPool get_connection()" << std::endl;
  return connections_.front();
}

void PgConnectionPool::selectQuery() {}

pqxx::result PgConnectionPool::selectQuery(const std::string &table) {
  std::string query = "SELECT * FROM " + table + ";";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec(query);
    transaction.commit();
    return result;
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in selectQuery: " << e.what() << std::endl;
    throw;
  }
}

bool PgConnectionPool::selectQuery(const std::string &table,
                                   const std::string &username) {
  const std::string query =
      "SELECT * FROM " + table + " WHERE username = '" + username + "'";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec(query);
    transaction.commit();
    return !result.empty(); // Return true if the result is not empty (user
                            // exists)
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in selectQuery: " << e.what() << std::endl;
    throw;
  }
}

pqxx::result PgConnectionPool::selectQuery(const std::string &table,
                                           const std::string &username,
                                           const std::string &password) {
  const std::string query = "SELECT * FROM " + table + " WHERE username = '" +
                            username + "' AND password = '" + password + "';";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec(query);
    transaction.commit();
    return result;
  } catch (const std::exception &e) {
    std::cerr << "Execption caught in selectLogin" << e.what() << std::endl;
    throw;
  }
}

bool PgConnectionPool::insertQuery(const std::string &table,
                                   const std::string &username,
                                   const std::string &password) {
  // INSERT INTO example_table (username, password) VALUES ('admin',
  // 'password')
  if (selectQuery(table, username)) {
    std::cerr << "User already exists" << std::endl;
    return false;
  }
  const std::string query = "INSERT INTO " + table +
                            "(username, password) VALUES ('" + username +
                            "', '" + password + "')";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    transaction.exec(query);
    transaction.commit(); // Commit the transaction to persist changes
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in insertRegistration: " << e.what()
              << std::endl;
    return false; // Return false to indicate failure
  }
}
