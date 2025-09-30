#include "oms_paper.hpp"
#include <cstdio>

namespace asio = boost::asio;

static asio::awaitable<void> track_ticks(OmsPaper *self) {
  for (;;) {
    auto t = co_await self->tick_tap.async_receive(asio::use_awaitable);
    self->last_px.store(t.last, std::memory_order_relaxed);
  }
}

static asio::awaitable<void> process_orders(OmsPaper *self) {
  for (;;) {
    auto r = co_await self->in_orders.async_receive(asio::use_awaitable);

    double px = (r.type == OrdType::Market)
                    ? self->last_px.load(std::memory_order_relaxed)
                    : r.px;

    std::printf("Order filled id=%s qty=%f px=%f\n", r.id.c_str(), r.qty, px);

    OrderResp resp{r.id, OrdStatus::Filled, r.qty, px, nullptr, r.ts_ns};
    co_await self->out_execs.async_send({}, resp, asio::use_awaitable);
  }
}

void OmsPaper::start() {
  asio::co_spawn(ex, track_ticks(this), asio::detached);
  asio::co_spawn(ex, process_orders(this), asio::detached);
}
