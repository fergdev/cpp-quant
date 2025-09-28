#include "ws_server.hpp"
#include <boost/beast/core.hpp>
#include <nlohmann/json.hpp>
#include <thread>

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

WsServer::WsServer(asio::io_context &io, unsigned short port,
                   Channel<Tick, 8192> &t, Channel<OrderResp, 4096> &e)
    : ioc(io), acc(io, tcp::endpoint(tcp::v4(), port)), in_ticks(t),
      in_exec(e) {}

void WsServer::start() {
  auto accept_once = [this](auto &&self) -> void {
    acc.async_accept([this, self](beast::error_code ec, tcp::socket s) {
      if (!ec) {
        auto ws = std::make_shared<beast::websocket::stream<tcp::socket>>(
            std::move(s));
        ws->async_accept([this, ws](beast::error_code ec2) {
          if (!ec2)
            clients.push_back(ws);
        });
      }
      self(self);
    });
  };
  accept_once(accept_once);

  std::thread([this] {
    Tick t;
    OrderResp e;
    auto broadcast = [this](const std::string &msg) {
      for (auto it = clients.begin(); it != clients.end();) {
        auto ws = *it;
        ws->async_write(asio::buffer(msg),
                        [this, it](beast::error_code ec, std::size_t) {
                          if (ec)
                            clients.erase(it);
                        });
        ++it;
      }
    };
    for (;;) {
      bool did = false;
      if (in_ticks.pop(t)) {
        did = true;
        broadcast(json{
            {"type", "tick"}, {"sym", t.sym}, {"px", t.last}, {"ts", t.ts_ns}}
                      .dump());
      }
      if (in_exec.pop(e)) {
        did = true;
        broadcast(json{
            {"type", "fill"},
            {"id", e.id},
            {"qty", e.filled_qty},
            {"px", e.avg_px},
            {"ts",
             e.ts_ns}}.dump());
      }
      if (!did)
        std::this_thread::yield();
    }
  }).detach();
}
