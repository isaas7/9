#include "../include/pgconnectionpool.hpp"
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

pqxx::result PgConnectionPool::executeSelect(const std::string &query) {
    try {
      auto connection = get_connection();
      pqxx::work transaction(*connection);
      pqxx::result result = transaction.exec(query);
      transaction.commit();
      return result;
    } catch (const std::exception &e) {
      std::cerr << "Exception caught in executeSelect: " << e.what()
                << std::endl;
      throw;
    }
  }

