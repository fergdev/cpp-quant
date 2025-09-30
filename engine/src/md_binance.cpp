#include "md_binance.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <chrono>
#include <cstdio>
#include <nlohmann/json.hpp>

#include <opentelemetry/trace/provider.h>

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

static asio::awaitable<void> md_once(MdBinance *self) {
  tcp::resolver res{self->ioc};
  auto r =
      co_await res.async_resolve(self->host, self->port, asio::use_awaitable);

  beast::ssl_stream<beast::tcp_stream> tls{self->ioc, self->ssl_ctx};
  co_await beast::get_lowest_layer(tls).async_connect(*r.begin(),
                                                      asio::use_awaitable);

  if (!::SSL_set_tlsext_host_name(tls.native_handle(), self->host.c_str()))
    throw std::runtime_error("SNI failed");

  co_await tls.async_handshake(ssl::stream_base::client, asio::use_awaitable);

  beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws{
      std::move(tls)};
  ws.set_option(beast::websocket::stream_base::timeout::suggested(
      beast::role_type::client));
  co_await ws.async_handshake(self->host, self->path, asio::use_awaitable);
  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(
      "quant-engine");

  beast::flat_buffer buf;
  for (;;) {
    buf.clear();
    co_await ws.async_read(buf, asio::use_awaitable);

    const auto s = beast::buffers_to_string(buf.cdata());
    const auto j = json::parse(s, nullptr, false);
    if (!j.is_object())
      continue;

    Tick t{j.value("s", "BTCUSDT"),
           std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::system_clock::now().time_since_epoch())
               .count(),
           std::stod(j.value("c", "0")),
           std::stod(j.value("b", "0")),
           std::stod(j.value("a", "0")),
           std::stod(j.value("v", "0"))};
    printf("[md] %s last=%.4f bid=%.4f ask=%.4f vol24=%.4f\n", t.sym.c_str(),
           t.last, t.bid, t.ask, t.vol24);

    auto span = tracer->StartSpan("md.publish_tick");
    {
      auto scope = tracer->WithActiveSpan(span);
      co_await self->out.async_send({}, t, asio::use_awaitable);
    }
  }
  co_return;
}

static asio::awaitable<void> md_loop(MdBinance *self) {
  using namespace std::chrono_literals;

  for (int attempt = 0;; ++attempt) {
    try {
      co_await md_once(self);
      attempt = 0;
    } catch (const std::exception &e) {
      std::fprintf(stderr, "[md] error: %s\n", e.what());
    } catch (...) {
      std::fprintf(stderr, "[md] unknown error\n");
    }

    auto backoff = 200ms * (1 << std::min(attempt, 5));
    auto delay = std::min<std::chrono::milliseconds>(5s, backoff);
    asio::steady_timer t{co_await asio::this_coro::executor};
    t.expires_after(delay);
    co_await t.async_wait(asio::use_awaitable);
  }
}

MdBinance::MdBinance(asio::io_context &io, ssl::context &ssl,
                     AsioChan<Tick> &out)
    : ioc(io), ssl_ctx(ssl), out(out) {}

void MdBinance::start() { asio::co_spawn(ioc, md_loop(this), asio::detached); }
