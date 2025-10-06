#pragma once
#include "config.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

inline const char *envOrNull(const char *k) {
  const char *v = std::getenv(k);
  return (v && *v) ? v : nullptr;
}

inline bool loadFromJsonFile(EngineConfig &c, const std::string &path) {
  std::ifstream f(path);
  if (!f) {
    std::cerr << "Could not open config file: " << path << std::endl;
    return false;
  }
  nlohmann::json j;
  f >> j;
  auto get = [&](auto key, auto &field) {
    if (j.contains(key))
      field = j.at(key).template get<std::decay_t<decltype(field)>>();
  };
  get("md_host", c.md_host);
  get("md_port", c.md_port);
  get("md_path", c.md_path);
  get("md_tls", c.md_tls);
  get("ws_port", c.ws_port);
  get("sma_window", c.sma_window);
  get("buy_threshold", c.buy_threshold);
  get("sell_threshold", c.sell_threshold);
  get("otlp_enable", c.otlp_enable);
  get("otlp_http_endpoint", c.otlp_http_endpoint);

  get("binance_api_key", c.binance_api_key);
  get("binance_secret", c.binance_secret);
  get("binance_host", c.binance_host);
  get("binance_recv_window", c.binance_recv_window);

  return true;
}
