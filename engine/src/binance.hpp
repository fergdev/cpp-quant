#pragma once
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <nlohmann/json.hpp>
#include <openssl/hmac.h>
#include <string>
#include <string_view>
#include <vector>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

inline int64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

inline std::string to_hex(const unsigned char *data, size_t len) {
  static const char *hexdig = "0123456789abcdef";
  std::string out;
  out.resize(len * 2);
  for (size_t i = 0; i < len; i++) {
    out[2 * i] = hexdig[(data[i] >> 4) & 0xF];
    out[2 * i + 1] = hexdig[(data[i]) & 0xF];
  }
  return out;
}

inline std::string hmac_sha256_hex(std::string_view key, std::string_view msg) {
  unsigned int len = EVP_MAX_MD_SIZE;
  unsigned char mac[EVP_MAX_MD_SIZE];
  HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
       reinterpret_cast<const unsigned char *>(msg.data()), msg.size(), mac,
       &len);
  return to_hex(mac, len);
}

inline std::string
make_qs(const std::vector<std::pair<std::string, std::string>> &kvs) {
  std::string qs;
  for (size_t i = 0; i < kvs.size(); ++i) {
    if (i)
      qs += "&";
    qs += kvs[i].first;
    qs += "=";
    qs += kvs[i].second;
  }
  return qs;
}
