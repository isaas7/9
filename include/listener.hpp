#ifndef LISTENER_HPP
#define LISTENER_HPP
#include "handler.hpp"
#include "message.hpp"
#include "session.hpp"
class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  std::shared_ptr<MessageService> messageService_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           std::shared_ptr<MessageService> messageService);
  void run();
  void fail(beast::error_code ec, char const *what);

private:
  std::shared_ptr<session> session_;
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif
