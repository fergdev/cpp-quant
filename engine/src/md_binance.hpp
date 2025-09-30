#pragma once
#include "bus.hpp"
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

struct MdBinance {
  asio::io_context &ioc;
  asio::ssl::context &ssl_ctx;
  std::string host = "stream.binance.com";
  std::string port = "9443";
  std::string path = "/ws/btcusdt@ticker";
  AsioChan<Tick> &out;

  MdBinance(asio::io_context &io, asio::ssl::context &ssl, AsioChan<Tick> &out);
  void start();
};
