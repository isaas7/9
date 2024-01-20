#include <pqxx/pqxx>
#include <string>
#include <vector>

struct dbSchema {
  std::vector<std::string> tables_;
  std::vector<std::string> columns_;
};

class PgConnectionPool {
public:
  PgConnectionPool(const std::string &conn_str, size_t pool_size, const dbSchema& schema);
  std::shared_ptr<pqxx::connection> get_connection();

  void selectQuery();
  bool selectQuery(const std::string &username);
  pqxx::result selectQuery(const std::string &username,
                           const std::string &password);
  void insertQuery();
  bool insertQuery(const std::string &username,
                   const std::string &password);
private:
  const dbSchema schema_;
  std::string conn_str_;
  size_t pool_size_;
  std::vector<std::shared_ptr<pqxx::connection>> connections_;
};

class SessionData {
public:
  std::string username;
  std::chrono::steady_clock::time_point expiration_time;
};
