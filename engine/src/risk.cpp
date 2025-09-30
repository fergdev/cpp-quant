#include "risk.hpp"
#include <boost/asio.hpp>
#include <cmath>

namespace asio = boost::asio;

static OrderReq make_order(const Signal &s) {
  OrderReq r{};
  r.id = s.strat + "-" + std::to_string(s.ts_ns);
  r.ts_ns = s.ts_ns;
  r.qty = s.qty;
  r.type = OrdType::Market;
  return r;
}

static asio::awaitable<void> run_risk(Risk *self) {
  for (;;) {
    auto sig = co_await self->in_signals.async_receive(asio::use_awaitable);

    if (!std::isfinite(sig.qty) || sig.qty == 0.0) {
      continue;
    }

    // TODO: place your risk checks here:
    // - max notional per order
    // - per-symbol position limits
    // - rate limiting / cool-down
    // - min tick size, etc.

    OrderReq req = make_order(sig);
    std::printf("[risk] pass id=%s qty=%f px=%f\n", req.id.c_str(), req.qty,
                req.px);

    co_await self->out_orders.async_send({}, req, asio::use_awaitable);
  }
}

void Risk::start() {
  auto ex = in_signals.get_executor();
  asio::co_spawn(ex, run_risk(this), asio::detached);
}
