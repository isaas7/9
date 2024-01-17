#include "include/listener.hpp"
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

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: http-server-async <address> <port> <threads>\n"
              << "Example:\n"
              << "    http-server-async 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }
  PgConnectionPool pg_pool("dbname=database_2024 user=user "
                           "password=password hostaddr=172.19.0.2 port=5432",
                           5);
  std::unordered_map<std::string, SessionData> session_storage;
  auto const address = net::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const threads = std::max<int>(1, std::atoi(argv[3]));

  net::io_context ioc{threads};
  std::make_shared<listener>(ioc, tcp::endpoint{address, port}, pg_pool,
                             session_storage)
      ->run();
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();
  return EXIT_SUCCESS;
}
