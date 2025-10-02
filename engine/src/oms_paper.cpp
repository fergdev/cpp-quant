#include "oms_paper.hpp"
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <cstdint>
#include <cstdlib>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;

static std::string env_or(const char *k, const char *def) {
  if (const char *v = std::getenv(k))
    return std::string(v);
  return std::string(def);
}
static uint16_t env_u16_or(const char *k, uint16_t def) {
  if (const char *v = std::getenv(k))
    return static_cast<uint16_t>(std::stoi(v));
  return def;
}
static int env_i_or(const char *k, int def) {
  if (const char *v = std::getenv(k))
    return std::stoi(v);
  return def;
}
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

  co_return nlohmann::json::parse(resp.body(), nullptr,
                                  /*allow_exceptions*/ true);
}

void OmsPaper::start() {
  asio::co_spawn(
      ex,
      [this]() -> asio::awaitable<void> {
        for (;;) {
          boost::system::error_code ec;
          Tick t = co_await tick_tap.async_receive(asio::use_awaitable);
          last_px.store(t.last, std::memory_order_relaxed);
        }
      },
      asio::detached);

  asio::co_spawn(
      ex,
      [this]() -> asio::awaitable<void> {
        const std::string host = env_or("BT_OMS_HOST", "127.0.0.1");
        const uint16_t port = env_u16_or("BT_OMS_PORT", 9000);
        const std::string path = env_or("BT_OMS_PATH", "/orders");
        const int t_ms = env_i_or("BT_OMS_TIMEOUT_MS", 2000);

        for (;;) {
          OrderReq req = co_await in_orders.async_receive(asio::use_awaitable);
          double px_hint = last_px.load(std::memory_order_relaxed);

          nlohmann::json jreq = {
              {"id", req.id},
              {"sym", req.sym},
              {"side", (req.side == Side::Buy ? "Buy" : "Sell")},
              {"type", (req.type == OrdType::Market ? "Market" : "Limit")},
              {"qty", req.qty},
              {"px", req.px},
              {"ts_ns", req.ts_ns},
              {"last_px_hint", px_hint}};

          OrderResp out{};
          out.id = req.id;
          out.ts_ns = req.ts_ns;

          try {

            auto resp = co_await post_json(host, port, path, jreq);
            out.st = OrdStatus::Filled;
            out.filled_qty = resp.value("filled_qty", req.qty);
            out.avg_px = resp.value("avg_px", req.px);
            out.err.clear();
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
