#include <pqxx/pqxx>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
struct dbSchema_ {
  std::unordered_map<std::string, std::vector<std::string>> db_schema;
};

class SessionData {
public:
  std::string username;
  std::chrono::steady_clock::time_point expiration_time;
};

class PgConnectionPool {
public:
  std::string generateSession();
  PgConnectionPool(const std::string &conn_str, size_t pool_size,
                   const dbSchema_ &schema);
  std::shared_ptr<pqxx::connection> get_connection();

  void selectQuery();
  pqxx::result selectQuery(const std::string &table);
  bool selectQuery(const std::string &table, const std::string &username);
  bool selectQuery(const std::string &table, const std::string &username,
                   const std::string &password);
  void insertQuery();
  bool insertQuery(const std::string &table, const std::string &username,
                   const std::string &password);

private:
  const dbSchema_ schema_;
  std::string conn_str_;
  size_t pool_size_;
  std::vector<std::shared_ptr<pqxx::connection>> connections_;
  std::unordered_map<std::string, SessionData> session_storage;
};
