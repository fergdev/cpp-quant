#pragma once
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>

namespace asio = boost::asio;

template <class T>
using AsioChan =
    asio::experimental::channel<void(boost::system::error_code, T)>;
struct Bus {
  AsioChan<Tick> ticks;
  AsioChan<Signal> signals;
  AsioChan<OrderReq> order_reqs;
  AsioChan<OrderResp> execs;

  Bus(asio::any_io_executor ex, std::size_t tick_cap = 8192,
      std::size_t sig_cap = 4096, std::size_t ord_cap = 4096,
      std::size_t exec_cap = 4096)
      : ticks(ex, tick_cap), signals(ex, sig_cap), order_reqs(ex, ord_cap),
        execs(ex, exec_cap) {}
};
