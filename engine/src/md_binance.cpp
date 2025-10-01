#include "md_binance.hpp"

#include <algorithm>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <chrono>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <string>

#include <opentelemetry/trace/provider.h>

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

template <class WebSocket>
static asio::awaitable<void> run_ws_loop(MdBinance *self, WebSocket &ws) {
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
    const auto j = json::parse(s, nullptr, /*allow_exceptions=*/false);
    if (!j.is_object())
      continue;

    auto get_num = [&](const char *k) -> double {
      if (j.contains(k)) {
        if (j[k].is_number())
          return j[k].get<double>();
        if (j[k].is_string())
          return std::stod(j[k].get<std::string>());
      }
      return 0.0;
    };

    Tick t{
        j.value("s", std::string{"BTCUSDT"}),
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count(),
        get_num("c"),
        get_num("b"),
        get_num("a"),
        get_num("v"),
    };

    std::printf("[md] %s last=%.4f bid=%.4f ask=%.4f vol24=%.4f\n",
                t.sym.c_str(), t.last, t.bid, t.ask, t.vol24);

    auto span = tracer->StartSpan("md.publish_tick");
    {
      auto scope = tracer->WithActiveSpan(span);
      co_await self->out.async_send({}, t, asio::use_awaitable);
    }
  }
}

static asio::awaitable<void> md_once(MdBinance *self) {
  tcp::resolver res{self->ioc};

  const std::string port = std::to_string(self->port);
  auto results =
      co_await res.async_resolve(self->host, port, asio::use_awaitable);

  if (self->use_tls) {
    beast::ssl_stream<beast::tcp_stream> tls{self->ioc, self->ssl_ctx};

    co_await beast::get_lowest_layer(tls).async_connect(*results.begin(),
                                                        asio::use_awaitable);

    if (!::SSL_set_tlsext_host_name(tls.native_handle(), self->host.c_str()))
      throw std::runtime_error("SNI failed");

    co_await tls.async_handshake(ssl::stream_base::client, asio::use_awaitable);

    beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws{
        std::move(tls)};
    co_await run_ws_loop(self, ws);

  } else {
    beast::tcp_stream tcp{self->ioc};
    co_await tcp.async_connect(*results.begin(), asio::use_awaitable);

    beast::websocket::stream<beast::tcp_stream> ws{std::move(tcp)};
    co_await run_ws_loop(self, ws);
  }

  co_return;
}

static asio::awaitable<void> md_loop(MdBinance *self) {
  using namespace std::chrono;
  int attempt = 0;

  for (;;) {
    try {
      co_await md_once(self);
      attempt = 0;
    } catch (const std::exception &e) {
      std::fprintf(stderr, "[md] error: %s\n", e.what());
      attempt = std::min(attempt + 1, 10);
    } catch (...) {
      std::fprintf(stderr, "[md] unknown error\n");
      attempt = std::min(attempt + 1, 10);
    }

    const auto base = 200ms;
    const auto cap = 5s;
    const auto factor = 1 << attempt;
    const auto backoff = base * factor;
    const auto delay = (backoff > cap) ? cap : backoff;

    asio::steady_timer t{co_await asio::this_coro::executor};
    t.expires_after(delay);
    co_await t.async_wait(asio::use_awaitable);
  }
}

MdBinance::MdBinance(asio::io_context &io, ssl::context &ssl,
                     AsioChan<Tick> &out, std::string host, uint16_t port,
                     std::string path, bool use_tls)
    : ioc(io), ssl_ctx(ssl), out(out), host(std::move(host)), port(port),
      path(std::move(path)), use_tls(use_tls) {}

void MdBinance::start() { asio::co_spawn(ioc, md_loop(this), asio::detached); }
