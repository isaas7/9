#include "../../include/listener.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
class HttpServerTest : public ::testing::Test {
protected:
  static constexpr int numThreads = 6;
  static constexpr int totalRequests = 80000;
  static constexpr double numRequests = totalRequests/numThreads;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HttpServerTest, ConcurrentRequests) {
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back([this, i] {
      for (int j = 0; j < totalRequests/numThreads; ++j) {
        std::cout << "Thread id: " << i << " Request id: " << j << std::endl;
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
                        ASSERT_EQ(response.result_int(), 200);
                        ASSERT_NE(
                            response.body().find("The resource returns \"/\""),
                            std::string::npos);
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
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
