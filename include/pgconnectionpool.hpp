#include <pqxx/pqxx>
#include <string>
#include <vector>
class PgConnectionPool {
public:
  PgConnectionPool(const std::string &conn_str, size_t pool_size);
  std::shared_ptr<pqxx::connection> get_connection();
  pqxx::result executeSelect(const std::string &query);

private:
  std::string conn_str_;
  size_t pool_size_;
  std::vector<std::shared_ptr<pqxx::connection>> connections_;
};

class SessionData {
public:
  std::string username;
  std::chrono::steady_clock::time_point expiration_time;
};
