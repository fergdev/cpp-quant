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
  boost::asio::io_context &ioc;
  boost::asio::ssl::context &ssl_ctx;
  Channel<Tick, 8192> &out;
  std::string host = "stream.binance.com", port = "9443",
              path = "/ws/btcusdt@ticker";

  MdBinance(boost::asio::io_context &io, boost::asio::ssl::context &ssl,
            Channel<Tick, 8192> &out);

  void start(); // launches async reader
};
