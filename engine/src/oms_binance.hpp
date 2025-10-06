#pragma once
#include "binance.hpp"
#include "binance_http.hpp"
#include "types.hpp"
#include <spdlog/spdlog.h>

inline asio::awaitable<OrderResp>
binance_place_order_spot(const OrderReq &req, double last_px_hint,
                         std::string api_key, std::string secret,
                         std::string host, unsigned recv_window) {

  spdlog::info("binance_place_order_spot");
  static std::atomic<int64_t> skew_ms{0};
  if (skew_ms.load() == 0) {
    try {
      skew_ms = co_await binance_time_skew_ms(host);
    } catch (...) {
      skew_ms = 0;
    }
  }

  const std::string side = (req.side == Side::Buy) ? "BUY" : "SELL";
  const std::string type = (req.type == OrdType::Market) ? "MARKET" : "LIMIT";

  std::vector<std::pair<std::string, std::string>> kv{
      {"symbol", req.sym},
      {"side", side},
      {"type", type},
      {"quantity", std::to_string(req.qty)},
      {"timestamp", std::to_string(now_ms() + skew_ms.load())},
      {"recvWindow", std::to_string(recv_window)},
      {"newClientOrderId", req.id}};

  if (type == "LIMIT") {
    kv.emplace_back("timeInForce", "GTC");
    kv.emplace_back("price", std::to_string(req.px));
  }

  const std::string qs = make_qs(kv);
  const std::string sig = hmac_sha256_hex(secret, qs);
  const std::string body = qs + "&signature=" + sig;

  const std::string target = "/api/v3/order";
  auto j = co_await http_json_request(host, target, http::verb::post, api_key,
                                      body, true, true, 443);

  OrderResp out{};
  out.id = req.id;
  out.ts_ns = req.ts_ns;
  out.st = OrdStatus::Filled;
  out.filled_qty = std::stod(j.value("executedQty", "0"));

  if (j.contains("fills") && j["fills"].is_array() && !j["fills"].empty()) {
    double sum_px = 0.0, sum_qty = 0.0;
    for (auto &f : j["fills"]) {
      double px = std::stod(f.value("price", "0"));
      double qty = std::stod(f.value("qty", "0"));
      sum_px += px * qty;
      sum_qty += qty;
    }
    out.avg_px = (sum_qty > 0 ? sum_px / sum_qty : 0.0);
  } else {
    out.avg_px = std::stod(j.value("price", "0"));
    if (out.avg_px == 0.0)
      out.avg_px = (req.type == OrdType::Market ? last_px_hint : req.px);
  }
  out.err.clear();
  co_return out;
}
