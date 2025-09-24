#pragma once
#include "bus.hpp"
#include "types.hpp"

struct Risk {
  Channel<Signal, 4096> &in;
  Channel<OrderReq, 4096> &out;
  double max_pos = 0.5;
  double pos = 0.0;

  Risk(Channel<Signal, 4096> &i, Channel<OrderReq, 4096> &o);
  void run();
};
