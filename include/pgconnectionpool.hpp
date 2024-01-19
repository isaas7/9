#include <pqxx/pqxx>
#include <string>
#include <vector>
class PgConnectionPool {
public:
  PgConnectionPool(const std::string &conn_str, size_t pool_size);
  std::shared_ptr<pqxx::connection> get_connection();

  void selectQuery();
  bool selectQuery(const std::string &table, const std::string &username);
  pqxx::result selectQuery(const std::string &table);
  pqxx::result selectQuery(const std::string &table,
                           const std::string &username,
                           const std::string &password);
  void insertQuery();
  bool insertQuery(const std::string &table, const std::string &username,
                   const std::string &password);

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
