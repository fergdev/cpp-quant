#include "oms_paper.hpp"
#include <thread>

OmsPaper::OmsPaper(Channel<OrderReq,4096>& i,
                   Channel<OrderResp,4096>& o,
                   Channel<Tick,8192>& tap)
  : in(i), out_exec(o), tick_tap(tap) {}

void OmsPaper::run() {
  // keep last_px updated
  std::thread([this]{
    Tick t;
    for(;;){
      if (tick_tap.pop(t)) last_px.store(t.last, std::memory_order_relaxed);
      else std::this_thread::yield();
    }
  }).detach();

  OrderReq r;
  for (;;) {
    if (!in.pop(r)) { std::this_thread::yield(); continue; }
    double px = (r.type == OrdType::Market) ? last_px.load(std::memory_order_relaxed) : r.px;
    out_exec.push(OrderResp{
      r.id,
      OrdStatus::Filled,
      r.qty,         // filled_qty
      px,            // avg_px
      nullptr,       // reason
      r.ts_ns
    });
  }
}
