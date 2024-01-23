#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "session.hpp"

class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint);

  void run();

  void fail(beast::error_code ec, char const *what);

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif
