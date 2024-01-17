#ifndef SESSION_HPP
#define SESSION_HPP
#include "handler.hpp"

class session : public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  PgConnectionPool &pg_pool_;
  std::unordered_map<std::string, SessionData> session_storage_;

public:
  session(tcp::socket &&socket, PgConnectionPool &pg_pool,
          std::unordered_map<std::string, SessionData> &session_storage)
      : stream_(std::move(socket)), pg_pool_(pg_pool),
        session_storage_(session_storage) {}
  void run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&session::do_read, shared_from_this()));
  }
  void do_read() {
    req_ = {};
    stream_.expires_after(std::chrono::seconds(30));
    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&session::on_read, shared_from_this()));
  }
  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec == http::error::end_of_stream)
      return do_close();
    if (ec)
      return fail(ec, "read");
    send_response(handle_request(std::move(req_), pg_pool_, session_storage_));
  }
  void send_response(http::message_generator &&msg) {
    bool keep_alive = msg.keep_alive();
    beast::async_write(stream_, std::move(msg),
                       beast::bind_front_handler(
                           &session::on_write, shared_from_this(), keep_alive));
  }
  void on_write(bool keep_alive, beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (ec)
      return fail(ec, "write");
    if (!keep_alive) {
      return do_close();
    }
    do_read();
  }
  void do_close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  }
  void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
  }
};
#endif
