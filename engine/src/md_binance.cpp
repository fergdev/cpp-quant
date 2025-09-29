#include "md_binance.hpp"
#include <chrono>
#include <cstdio>
#include <opentelemetry/trace/provider.h>

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

static asio::awaitable<void> md_loop(MdBinance *self) {
  try {
    tcp::resolver res{self->ioc};
    auto r =
        co_await res.async_resolve(self->host, self->port, asio::use_awaitable);

    beast::ssl_stream<beast::tcp_stream> tls{self->ioc, self->ssl_ctx};
    co_await beast::get_lowest_layer(tls).async_connect(*r.begin(),
                                                        asio::use_awaitable);
    if (!SSL_set_tlsext_host_name(tls.native_handle(), self->host.c_str()))
      throw std::runtime_error("SNI");
    co_await tls.async_handshake(ssl::stream_base::client, asio::use_awaitable);

    beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws{
        std::move(tls)};
    ws.set_option(beast::websocket::stream_base::timeout::suggested(
        beast::role_type::client));
    co_await ws.async_handshake(self->host, self->path, asio::use_awaitable);

    beast::flat_buffer buf;
    for (;;) {
      buf.clear();
      co_await ws.async_read(buf, asio::use_awaitable);
      auto s = beast::buffers_to_string(buf.cdata());
      auto j = json::parse(s, nullptr, false);
      if (!j.is_object())
        continue;
      auto tracer =
          opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(
              "quant-engine");
      auto span = tracer->StartSpan("engine_boot");
      {
        auto scope = tracer->WithActiveSpan(span);

        Tick t{j.value("s", "BTCUSDT"),
               std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count(),
               std::stod(j.value("c", "0")),
               std::stod(j.value("b", "0")),
               std::stod(j.value("a", "0")),
               std::stod(j.value("v", "0"))};
        printf("Tick ask: %f\n", t.ask);
        (void)self->out.push(t);
      }
    }
  } catch (std::exception const &e) {
    std::fprintf(stderr, "[md] %s\n", e.what());
  }
  co_return;
}

MdBinance::MdBinance(asio::io_context &io, ssl::context &ssl,
                     Channel<Tick, 8192> &out)
    : ioc(io), ssl_ctx(ssl), out(out) {}

void MdBinance::start() { asio::co_spawn(ioc, md_loop(this), asio::detached); }
