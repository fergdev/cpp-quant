#include "risk.hpp"
#include <cmath>
#include <thread>

Risk::Risk(Channel<Signal,4096>& i, Channel<OrderReq,4096>& o) : in(i), out(o) {}

void Risk::run() {
  Signal s;
  for (;;) {
    if (!in.pop(s)) { std::this_thread::yield(); continue; }
    double delta = (s.side == Side::Buy ? +s.qty : -s.qty);
    if (std::abs(pos + delta) > max_pos) continue;
    pos += delta;
    out.push(OrderReq{ s.strat + "-" + std::to_string(s.ts_ns), s.sym, s.side,
                       OrdType::Market, s.qty, 0.0, s.ts_ns });
  }
}
