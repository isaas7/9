#include "../include/session.hpp"

session::session(tcp::socket &&socket, PgConnectionPool &pg_pool)
    : stream_(std::move(socket)), pg_pool_(pg_pool) {}

void session::run() {
  net::dispatch(
      stream_.get_executor(),
      beast::bind_front_handler(&session::do_read, shared_from_this()));
}

void session::do_read() {
  req_ = {};
  stream_.expires_after(std::chrono::seconds(30));
  http::async_read(
      stream_, buffer_, req_,
      beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(beast::error_code ec, std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec == http::error::end_of_stream)
    return do_close();
  if (ec)
    return fail(ec, "read");
  send_response(handle_request(std::move(req_), pg_pool_));
}

void session::send_response(http::message_generator &&msg) {
  bool keep_alive = msg.keep_alive();
  beast::async_write(stream_, std::move(msg),
                     beast::bind_front_handler(&session::on_write,
                                               shared_from_this(), keep_alive));
}

void session::on_write(bool keep_alive, beast::error_code ec,
                       std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);
  if (ec)
    return fail(ec, "write");
  if (!keep_alive) {
    return do_close();
  }
  do_read();
}

void session::do_close() {
  beast::error_code ec;
  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}

void session::fail(beast::error_code ec, char const *what) {
  spdlog::error("session::fail {}: {}", what, ec.message());
}
