#include "ws_server.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using json = nlohmann::json;

static asio::awaitable<void> accept_loop(WsServer *self) {
  for (;;) {
    beast::error_code ec;
    tcp::socket sock = co_await self->acc.async_accept(
        asio::redirect_error(asio::use_awaitable, ec));
    if (ec)
      continue;

    auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(sock));
    ws->set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    co_await ws->async_accept(asio::redirect_error(asio::use_awaitable, ec));
    if (ec)
      continue;

    asio::dispatch(self->strand, [self, ws] {
      auto c = std::make_shared<WsServer::Client>();
      c->ws = ws;
      self->clients.push_back(c);
    });
  }
}

static asio::awaitable<void> pump_ticks(WsServer *self) {
  for (;;) {
    auto t = co_await self->tick_ch.async_receive(asio::use_awaitable);
    json j = {
        {"type", "tick"}, {"sym", t.sym}, {"px", t.last}, {"ts", t.ts_ns}};
    self->enqueue_broadcast(j.dump());
  }
}

static asio::awaitable<void> pump_execs(WsServer *self) {
  for (;;) {
    auto e = co_await self->exec_ch.async_receive(asio::use_awaitable);
    json j = {{"type", "fill"},
              {"id", e.id},
              {"qty", e.filled_qty},
              {"px", e.avg_px},
              {"ts", e.ts_ns}};
    self->enqueue_broadcast(j.dump());
  }
}

void WsServer::enqueue_broadcast(std::string msg) {
  asio::dispatch(strand, [this, m = std::move(msg)]() mutable {
    for (auto it = clients.begin(); it != clients.end();) {
      auto &c = *it;
      if (!c || !c->ws) {
        it = clients.erase(it);
        continue;
      }
      c->pending.push_back(m);
      if (c->pending.size() == 1)
        start_write_locked(c);
      ++it;
    }
  });
}

void WsServer::start_write_locked(const std::shared_ptr<Client> &c) {
  auto &front = c->pending.front();
  auto buf = asio::buffer(front);
  c->ws->async_write(
      buf,
      asio::bind_executor(strand, [this, c](beast::error_code ec, std::size_t) {
        if (ec) {
          auto it = std::find_if(clients.begin(), clients.end(),
                                 [&](auto &p) { return p.get() == c.get(); });
          if (it != clients.end())
            clients.erase(it);
          return;
        }
        if (!c->pending.empty())
          c->pending.pop_front();
        if (!c->pending.empty())
          start_write_locked(c);
      }));
}

void WsServer::start() {
  asio::co_spawn(ioc, accept_loop(this), asio::detached);
  asio::co_spawn(ioc, pump_ticks(this), asio::detached);
  asio::co_spawn(ioc, pump_execs(this), asio::detached);
}
