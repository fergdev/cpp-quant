#pragma once
#include "types.hpp"
#include "bus.hpp"
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <list>
#include <memory>

struct WsServer {
  boost::asio::io_context& ioc;
  boost::asio::ip::tcp::acceptor acc;
  Channel<Tick,8192>& in_ticks;
  Channel<OrderResp,4096>& in_exec;
  std::list<std::shared_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>>> clients;

  WsServer(boost::asio::io_context& io, unsigned short port,
           Channel<Tick,8192>& t, Channel<OrderResp,4096>& e);
  void start();
};
