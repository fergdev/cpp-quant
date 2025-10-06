#pragma once
#include "binance.hpp"
#include <spdlog/spdlog.h>

namespace http = boost::beast::http;
namespace beast = boost::beast;
inline asio::awaitable<nlohmann::json>
http_json_request(std::string host, std::string target, http::verb method,
                  std::string api_key, std::string body_or_qs = "",
                  bool body_is_form = false, bool https = false,
                  unsigned port = 443) {
  spdlog::info("http_json_request");
  spdlog::info("req_body {}", body_or_qs);
  beast::flat_buffer buffer;
  auto exec = co_await asio::this_coro::executor;

  tcp::resolver res{exec};
  auto endpoints = co_await res.async_resolve("127.0.0.1", std::to_string(9000),
                                              asio::use_awaitable);
  // auto endpoints = co_await res.async_resolve(host, std::to_string(port),
  //                                             asio::use_awaitable);

  beast::tcp_stream stream{exec};
  spdlog::trace("Async connnect");
  co_await stream.async_connect(endpoints, asio::use_awaitable);

  spdlog::trace("sending request");
  http::request<http::string_body> req{method, target, 11};
  req.set(http::field::host, host);
  if (!api_key.empty())
    req.set("X-MBX-APIKEY", api_key);

  spdlog::info("isform {}", body_is_form);
  if (body_is_form) {
    req.set(http::field::content_type, "application/x-www-form-urlencoded");
    req.body() = std::move(body_or_qs);
    req.prepare_payload();
  } else if (!body_or_qs.empty()) {
    // query string included inside target already – keep body empty
  }

  http::response<http::string_body> resp;
  spdlog::info("writing request");
  co_await http::async_write(stream, req, asio::use_awaitable);
  spdlog::info("reading response");
  co_await http::async_read(stream, buffer, resp, asio::use_awaitable);

  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  if (resp.result() != http::status::ok) {
    throw std::runtime_error("HTTP " + std::to_string((unsigned)resp.result()) +
                             " " + resp.body());
  }

  co_return nlohmann::json::parse(resp.body(), nullptr, true);
}

inline asio::awaitable<int64_t> binance_time_skew_ms(std::string base_host) {
  auto j = co_await http_json_request(base_host, "/api/v3/time",
                                      http::verb::get, "");
  auto server_ms = j.value("serverTime", (int64_t)0);
  co_return server_ms - now_ms();
}
