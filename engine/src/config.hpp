#pragma once
#include <cstddef>
#include <string>

struct EngineConfig {
  std::string md_host = "stream.binance.com";
  uint16_t md_port = 9443;
  std::string md_path = "/ws/...";
  bool md_tls = true;

  uint16_t ws_port = 8080;

  int sma_window = 20;
  double buy_threshold = 1.001;
  double sell_threshold = 0.999;

  bool otlp_enable = false;
  std::string otlp_http_endpoint = "http://tempo:4318";

  std::string binance_api_key;
  std::string binance_secret;
  std::string binance_host;
  int binance_recv_window;
};
