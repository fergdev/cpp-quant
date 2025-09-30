#include "bus.hpp"
#include "md_binance.hpp"
#include "oms_paper.hpp"
#include "risk.hpp"
#include "strategy_sma.hpp"
#include "ws_server.hpp"
#include <boost/asio/ssl.hpp>

int main() {
  printf("Engine started\n");

  asio::io_context ioc;
  asio::ssl::context ssl_ctx{asio::ssl::context::tls_client};
  ssl_ctx.set_default_verify_paths();

  Bus bus{ioc.get_executor()};

  MdBinance md{ioc, ssl_ctx, bus.ticks};
  StratSMA strat{ioc.get_executor(), bus.ticks, bus.signals};
  Risk risk{bus.signals, bus.order_reqs};
  OmsPaper oms{ioc.get_executor(), bus.order_reqs, bus.execs, bus.ticks};
  WsServer ws{ioc, 8080, bus.ticks, bus.execs};

  md.start();
  strat.start();
  risk.start();
  oms.start();
  ws.start();

  std::vector<std::thread> pool;
  for (int i = 0; i < 2; ++i)
    pool.emplace_back([&] { ioc.run(); });
  for (auto &t : pool)
    t.join();
}
