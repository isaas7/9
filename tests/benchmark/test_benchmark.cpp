#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <chrono>
#include <iostream>
#include <thread>

class HttpServerTest {
protected:
  int numThreads;
  int totalRequests;

public:
  HttpServerTest(int threads, int requests)
      : numThreads(threads), totalRequests(requests) {}

  void run() {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
      threads.emplace_back([this, i] {
        for (int j = 0; j < totalRequests / numThreads; ++j) {
          // std::cout << "Thread id: " << i << " Request id: " << j <<
          // std::endl;
          boost::asio::io_context ioContext;
          boost::asio::ip::tcp::resolver resolver(ioContext);
          boost::asio::ip::tcp::socket socket(ioContext);

          boost::beast::http::request<boost::beast::http::string_body> request;
          request.method(boost::beast::http::verb::get);
          request.target("/");

          resolver.async_resolve(
              "localhost", "8080",
              [&](const boost::system::error_code &ec, auto endpoints) {
                if (!ec) {
                  boost::asio::async_connect(
                      socket, endpoints,
                      [&](const boost::system::error_code &ec, const auto &) {
                        if (!ec) {
                          boost::beast::http::write(socket, request);

                          boost::beast::flat_buffer buffer;
                          boost::beast::http::response<
                              boost::beast::http::string_body>
                              response;
                          boost::beast::http::read(socket, buffer, response);
                          // std::cout << response.body() << std::endl;
                          // Add your assertions or further processing here
                        }
                      });
                }
              });

          ioContext.run();
        }
      });
    }

    for (auto &thread : threads) {
      thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
  }
};

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <numThreads> <totalRequests>"
              << std::endl;
    return 1;
  }

  int numThreads = std::stoi(argv[1]);
  int totalRequests = std::stoi(argv[2]);

  HttpServerTest test(numThreads, totalRequests);
  test.run();

  return 0;
}
