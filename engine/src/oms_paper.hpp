#pragma once
#include "types.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>

namespace asio = boost::asio;
template <class T>
using AsioChan =
    asio::experimental::channel<void(boost::system::error_code, T)>;

struct OmsPaper {
  asio::any_io_executor ex;
  AsioChan<OrderReq> &in_orders;
  AsioChan<OrderResp> &out_execs;
  AsioChan<Tick> &tick_tap;
  std::string binance_api_key;
  std::string binance_secret;
  std::string binance_host;
  int binance_recv_window;

  std::atomic<double> last_px{0.0};

  OmsPaper(asio::any_io_executor ex, AsioChan<OrderReq> &in,
           AsioChan<OrderResp> &out, AsioChan<Tick> &tap,
           std::string binance_api_key, std::string binance_secret,
           std::string binance_host, int binance_recv_window)
      : ex(ex), in_orders(in), out_execs(out), tick_tap(tap),
        binance_api_key(binance_api_key), binance_secret(binance_secret),
        binance_host(binance_host), binance_recv_window(binance_recv_window) {}

  void start();
};
