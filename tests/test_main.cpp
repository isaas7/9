#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

std::string bearer_token_;

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session> {
  tcp::resolver resolver_;
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_; // (Must persist between reads)
  http::request<http::empty_body> req_;
  http::response<http::string_body> res_;

public:
  // Objects are constructed with a strand to
  // ensure that handlers do not execute concurrently.
  explicit session(net::io_context &ioc)
      : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}

  // Start the asynchronous operation
  void run(char const *host, char const *port, char const *target,
           int version) {
    // Set up an HTTP GET request message
    req_.version(version);
    req_.method(http::verb::get);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Look up the domain name
    resolver_.async_resolve(
        host, port,
        beast::bind_front_handler(&session::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec)
      return fail(ec, "resolve");

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    stream_.async_connect(
        results,
        beast::bind_front_handler(&session::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type) {
    if (ec)
      return fail(ec, "connect");

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(
        stream_, req_,
        beast::bind_front_handler(&session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Receive the HTTP response
    http::async_read(
        stream_, buffer_, res_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "read");

    // Write the message to standard out
    std::cout << res_ << std::endl;
    auto it = res_.find(http::field::authorization);
    if (it != res_.end()) {
      bearer_token_ = std::string(it->value());
      std::cout << "Bearer token " << bearer_token_ << std::endl;
    } else {
      // Check for case-insensitive comparison
      it = res_.find(boost::beast::http::field::authorization);
      if (it != res_.end()) {
        bearer_token_ = std::string(it->value());
        std::cout << "Bearer token " << bearer_token_ << std::endl;
      }
    }

    // Gracefully close the socket
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected)
      return fail(ec, "shutdown");

    // If we get here then the connection is closed gracefully
  }
};

class HttpClientTest : public ::testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {
    // Clean up any common resources
  }

  // You can define helper functions or variables accessible to all tests
};

TEST_F(HttpClientTest, AsyncHttpRequest) {
  const char *host = "localhost";
  const char *port = "8080";
  const char *target = "/api?=customers";
  int version = 11;

  // The io_context is required for all I/O
  net::io_context ioc;

  // Use the retrieved Bearer token in the Authorization header
  std::string authorization_header = "Bearer session_1705409190456771984";

  // Create an HTTP request
  http::request<http::empty_body> req;
  req.version(version);
  req.method(http::verb::get);
  req.target(target);
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Set the Authorization header
  req.set(http::field::authorization, authorization_header);

  // Launch the asynchronous operation
  std::make_shared<session>(ioc)->run(host, port, target, version);

  // Run the I/O service. The call will return when
  // the get operation is complete.
  ioc.run();

  // Add your assertions based on the expected behavior of the HTTP client
  // For example, you can check if the response meets certain criteria
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
