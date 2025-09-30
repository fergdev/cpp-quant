#pragma once
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast/websocket.hpp>
#include <deque>
#include <memory>
#include <vector>

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;

template <class T>
using AsioChan =
    asio::experimental::channel<void(boost::system::error_code, T)>;

struct WsServer {
  asio::io_context &ioc;
  tcp::acceptor acc;
  AsioChan<Tick> &tick_ch;
  AsioChan<OrderResp> &exec_ch;

  asio::strand<asio::io_context::executor_type> strand;

  struct Client {
    std::shared_ptr<beast::websocket::stream<tcp::socket>> ws;
    std::deque<std::string> pending;
  };

  std::vector<std::shared_ptr<Client>> clients;

  WsServer(asio::io_context &io, unsigned short port, AsioChan<Tick> &t,
           AsioChan<OrderResp> &e)
      : ioc(io), acc(io, tcp::endpoint(tcp::v4(), port)), tick_ch(t),
        exec_ch(e), strand(io.get_executor()) {}

  void start();
  void enqueue_broadcast(std::string msg);

private:
  void start_write_locked(const std::shared_ptr<Client> &c);
};
