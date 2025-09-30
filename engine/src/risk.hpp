#pragma once
#include "bus.hpp"
#include "types.hpp"

struct Risk {
  AsioChan<Signal> &in_signals;
  AsioChan<OrderReq> &out_orders;
  void start();
};
