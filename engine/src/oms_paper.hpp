#pragma once
#include "bus.hpp"
#include "types.hpp"
#include <atomic>

struct OmsPaper {
  Channel<OrderReq, 4096> &in;
  Channel<OrderResp, 4096> &out_exec;
  Channel<Tick, 8192> &tick_tap;
  std::atomic<double> last_px{0.0};

  OmsPaper(Channel<OrderReq, 4096> &i, Channel<OrderResp, 4096> &o,
           Channel<Tick, 8192> &tap);
  void run();
};
