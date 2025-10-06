#include "oms_paper.hpp"
#include "oms_binance.hpp"
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;

asio::awaitable<nlohmann::json> post_json(std::string host, uint16_t port,
                                          std::string target,
                                          nlohmann::json body) {
  using tcp = asio::ip::tcp;

  beast::flat_buffer buffer;
  tcp::resolver res{co_await asio::this_coro::executor};

  auto results = co_await res.async_resolve(host, std::to_string(port),
                                            asio::use_awaitable);

  beast::tcp_stream stream{co_await asio::this_coro::executor};
  boost::system::error_code ec;
  co_await stream.async_connect(results,
                                asio::redirect_error(asio::use_awaitable, ec));
  if (ec)
    throw std::runtime_error("connect failed: " + ec.message());

  http::request<http::string_body> req{http::verb::post, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::content_type, "application/json");
  req.body() = body.dump();
  req.prepare_payload();

  http::response<http::string_body> resp;

  co_await http::async_write(stream, req,
                             asio::redirect_error(asio::use_awaitable, ec));
  if (ec)
    throw std::runtime_error("write failed: " + ec.message());

  co_await http::async_read(stream, buffer, resp,
                            asio::redirect_error(asio::use_awaitable, ec));
  if (ec)
    throw std::runtime_error("read failed: " + ec.message());

  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  if (resp.result() != http::status::ok) {
    throw std::runtime_error(
        "HTTP " + std::to_string(static_cast<unsigned>(resp.result())));
  }

  co_return nlohmann::json::parse(resp.body(), nullptr, true);
}

void OmsPaper::start() {
  asio::co_spawn(
      ex,
      [this]() -> asio::awaitable<void> {
        for (;;) {
          Tick t = co_await tick_tap.async_receive(asio::use_awaitable);
          last_px.store(t.last, std::memory_order_relaxed);
        }
      },
      asio::detached);

  asio::co_spawn(
      ex,
      [this]() -> asio::awaitable<void> {
        for (;;) {
          OrderReq req = co_await in_orders.async_receive(asio::use_awaitable);
          spdlog::info("Paper order '{}'", req.sym);
          double hint = last_px.load(std::memory_order_relaxed);

          OrderResp out{};
          out.id = req.id;
          out.ts_ns = req.ts_ns;

          try {
            out = co_await binance_place_order_spot(
                req, hint, binance_api_key, binance_secret, binance_host,
                binance_recv_window);
          } catch (const std::exception &e) {
            out.st = OrdStatus::Rejected;
            out.filled_qty = 0.0;
            out.avg_px = 0.0;
            out.err = e.what();
          }

          co_await out_execs.async_send({}, out, asio::use_awaitable);
        }
      },
      asio::detached);
}
