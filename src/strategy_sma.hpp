#pragma once
#include "types.hpp"
#include "bus.hpp"
#include <vector>

struct StratSMA {
  Channel<Tick,   8192>& in;
  Channel<Signal, 4096>& out;

  int w = 20;
  std::vector<double> buf;
  int   idx  = 0;
  bool  warm = false;
  double sum = 0.0;

  StratSMA(Channel<Tick,8192>& i, Channel<Signal,4096>& o);
  void run();
};
