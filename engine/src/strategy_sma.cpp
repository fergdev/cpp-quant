#include "strategy_sma.hpp"
#include <numeric>
#include <thread>

StratSMA::StratSMA(Channel<Tick,8192>& i, Channel<Signal,4096>& o)
  : in(i), out(o), buf(w, 0.0) {}

void StratSMA::run() {
  Tick t;
  for (;;) {
    if (!in.pop(t)) { std::this_thread::yield(); continue; }
    sum -= buf[idx]; buf[idx] = t.last; sum += t.last; idx = (idx + 1) % w;
    if (!warm && idx == 0) warm = true;
    if (!warm) continue;
    double sma = sum / w;
    if (t.last > sma * 1.001) out.push(Signal{"sma20", t.sym, Side::Buy, 0.01, 0.0, t.ts_ns});
    else if (t.last < sma * 0.999) out.push(Signal{"sma20", t.sym, Side::Sell, 0.01, 0.0, t.ts_ns});
  }
}
