#pragma once
#include "bus.hpp"
#include "types.hpp"
#include <sqlite3.h>

struct Persist {
  Channel<Tick, 8192> &ticks;
  Channel<OrderResp, 4096> &fills;
  sqlite3 *db{};

  Persist(Channel<Tick, 8192> &t, Channel<OrderResp, 4096> &f);
  void run();
};
