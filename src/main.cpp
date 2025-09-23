#include "types.hpp"
#include "bus.hpp"
#include "md_binance.hpp"
#include "strategy_sma.hpp"
#include "risk.hpp"
#include "oms_paper.hpp"
#include "persist_sqlite.hpp"
#include "ws_server.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>
#include <chrono>

int main(){
  namespace asio = boost::asio;
  asio::io_context ioc;
  asio::ssl::context ssl_ctx{asio::ssl::context::tls_client};
  ssl_ctx.set_default_verify_paths();

  Channel<Tick,8192> tick_q, tick_ws_q, tick_persist_q;
  Channel<Signal,4096> sig_q;
  Channel<OrderReq,4096> ord_q;
  Channel<OrderResp,4096> exec_q;

  MdBinance md{ioc, ssl_ctx, tick_q};
  StratSMA strat{tick_q, sig_q};
  Risk risk{sig_q, ord_q};
  OmsPaper oms{ord_q, exec_q, tick_q};
  Persist persist{tick_persist_q, exec_q};
  WsServer ws{ioc, 8080, tick_ws_q, exec_q};

  md.start();
  ws.start();

  std::thread([&]{
    Tick t;
    for(;;){
      if (tick_q.pop(t)) { tick_ws_q.push(t); tick_persist_q.push(t); }
      else std::this_thread::yield();
    }
  }).detach();

  std::thread([&]{ strat.run(); }).detach();
  std::thread([&]{ risk.run();  }).detach();
  std::thread([&]{ oms.run();   }).detach();
  std::thread([&]{ persist.run(); }).detach();

  std::vector<std::thread> pool;
  for (int i=0;i<2;++i) pool.emplace_back([&]{ ioc.run(); });
  for (auto& th : pool) th.join();
  return 0;
}
