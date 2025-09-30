#pragma once
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <vector>

namespace asio = boost::asio;
template <class T>
using AsioChan =
    asio::experimental::channel<void(boost::system::error_code, T)>;

struct StratSMA {
  asio::any_io_executor ex;
  AsioChan<Tick> &in_ticks;
  AsioChan<Signal> &out_signals;

  const int w;
  std::vector<double> buf;
  int idx = 0;
  bool warm = false;
  double sum = 0.0;

  StratSMA(asio::any_io_executor ex, AsioChan<Tick> &in, AsioChan<Signal> &out,
           int window = 20)
      : ex(ex), in_ticks(in), out_signals(out), w(window), buf(w, 0.0) {}

  void start();
};
