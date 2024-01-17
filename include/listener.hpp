#ifndef LISTENER_HPP
#define LISTENER_HPP
#include "session.hpp"
class listener : public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  PgConnectionPool &pg_pool_;
  std::unordered_map<std::string, SessionData> session_storage_;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           PgConnectionPool &pg_pool,
           std::unordered_map<std::string, SessionData> &session_storage)
      : ioc_(ioc), acceptor_(net::make_strand(ioc)), pg_pool_(pg_pool),
        session_storage_(session_storage) {
    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }
  void run() { do_accept(); }
  void fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
  }

private:
  void do_accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }
  void on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
      return;
    } else {
      std::make_shared<session>(std::move(socket), pg_pool_, session_storage_)
          ->run();
    }
    do_accept();
  }
};
#endif
