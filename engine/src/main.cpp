#include "bus.hpp"
#include "config.hpp"
#include "config_load.hpp"
#include "md_binance.hpp"
#include "oms_paper.hpp"
#include "risk.hpp"
#include "strategy_sma.hpp"
#include "ws_server.hpp"
#include <boost/asio/ssl.hpp>
#include <iostream>

int main(int argc, char **argv) {
  printf("Engine started\n");

  std::string explicit_config;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a.rfind("--config=", 0) == 0)
      explicit_config = a.substr(9);
  }

  if (explicit_config.empty()) {
    std::cerr << "No --config= specified" << std::endl;
    return -1;
  }

  EngineConfig config;
  if (!loadFromJsonFile(config, explicit_config)) {
    std::cerr << "Could not load config from " << explicit_config << std::endl;
    return -1;
  }

  asio::io_context ioc;
  asio::ssl::context ssl_ctx{asio::ssl::context::tls_client};
  ssl_ctx.set_default_verify_paths();

  Bus bus{ioc.get_executor()};

  MdBinance md{ioc,
               ssl_ctx,
               bus.ticks,
               config.md_host,
               config.md_port,
               config.md_path,
               config.md_tls};

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
