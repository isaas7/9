#ifndef LISTENER_HPP
#define LISTENER_HPP
#include "application.hpp"
#include "beast.hpp"
#include "net.hpp"
class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  ChatSystem chatSystem_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint, ChatSystem chatSystem);

  void run();

  void fail(beast::error_code ec, char const *what);

private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif
