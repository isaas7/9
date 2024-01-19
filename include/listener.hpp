#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "session.hpp"
#include <iostream>

class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  PgConnectionPool &pg_pool_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           PgConnectionPool &pg_pool);

  void run();

  void fail(beast::error_code ec, char const *what);

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif
