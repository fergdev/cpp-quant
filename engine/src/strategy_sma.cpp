#include "strategy_sma.hpp"
#include <cstdio>

namespace asio = boost::asio;

static asio::awaitable<void> run_sma(StratSMA *self) {
  for (;;) {
    auto t = co_await self->in_ticks.async_receive(asio::use_awaitable);

    self->sum -= self->buf[self->idx];
    self->buf[self->idx] = t.last;
    self->sum += t.last;
    self->idx = (self->idx + 1) % self->w;

    if (!self->warm && self->idx == 0)
      self->warm = true;
    if (!self->warm)
      continue;

    const double sma = self->sum / self->w;

    // simple band (±0.1%) -> tweak as needed
    if (t.last > sma * 1.001) {
      std::printf("[sma] BUY  sym=%s last=%.4f sma=%.4f\n", t.sym.c_str(),
                  t.last, sma);
      Signal s{"sma20", t.sym, Side::Buy, 0.01, 0.0, t.ts_ns};
      co_await self->out_signals.async_send({}, s, asio::use_awaitable);
    } else if (t.last < sma * 0.999) {
      std::printf("[sma] SELL sym=%s last=%.4f sma=%.4f\n", t.sym.c_str(),
                  t.last, sma);
      Signal s{"sma20", t.sym, Side::Sell, 0.01, 0.0, t.ts_ns};
      co_await self->out_signals.async_send({}, s, asio::use_awaitable);
    }
  }
}

void StratSMA::start() { asio::co_spawn(ex, run_sma(this), asio::detached); }
