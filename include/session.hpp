#ifndef SESSION_HPP
#define SESSION_HPP

#include "handler.hpp"

class session : public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  PgConnectionPool &pg_pool_;

public:
  session(tcp::socket &&socket, PgConnectionPool &pg_pool);

  void run();
  void do_read();
  void on_read(beast::error_code ec, std::size_t bytes_transferred);
  void send_response(http::message_generator &&msg);
  void on_write(bool keep_alive, beast::error_code ec,
                std::size_t bytes_transferred);
  void do_close();
  void fail(beast::error_code ec, char const *what);
};

#endif
