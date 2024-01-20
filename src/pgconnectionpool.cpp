#include "../include/pgconnectionpool.hpp"
#include <boost/asio/query.hpp>
#include <iostream>
PgConnectionPool::PgConnectionPool(const std::string &conn_str,
                                   size_t pool_size, const dbSchema &schema)
    : conn_str_(conn_str), pool_size_(pool_size), schema_(schema) {
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

bool PgConnectionPool::selectQuery(const std::string &username) {
  const std::string query = "SELECT * FROM " + schema_.tables_[0] + " WHERE " +
                            schema_.columns_[0] + " = $1";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec_params(query, username);
    transaction.commit();
    return !result.empty();
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in selectQuery: " << e.what() << std::endl;
    throw;
  }
}

pqxx::result PgConnectionPool::selectQuery(const std::string &username,
                                           const std::string &password) {
  const std::string query =
      "SELECT * FROM " + schema_.tables_[0] + " WHERE "+ schema_.columns_[0] +" = $1 AND "+ schema_.columns_[1] +" = $2"; 
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    pqxx::result result = transaction.exec_params(query, username, password);
    transaction.commit();
    return result;
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in selectLogin: " << e.what() << std::endl;
    throw;
  }
}

bool PgConnectionPool::insertQuery(const std::string &username,
                                   const std::string &password) {
  if (selectQuery(username)) {
    std::cerr << "Exception caught in insertQuery : User already exists"
              << std::endl;
    return false;
  }
  const std::string query =
      "INSERT INTO " + schema_.tables_[0] + "("+ schema_.columns_[0] +", "+ schema_.columns_[1] +") VALUES ($1, $2)";
  try {
    auto connection = get_connection();
    pqxx::work transaction(*connection);
    transaction.exec_params(query, username, password);
    transaction.commit();
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Exception caught in insertQuery: " << e.what() << std::endl;
    return false;
  }
}
