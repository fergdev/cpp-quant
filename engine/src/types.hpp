#pragma once
#include <cstdint>
#include <string>
struct Tick {
  std::string sym;
  int64_t ts_ns;
  double last, bid, ask, vol24;
};
enum class Side { Buy, Sell };
struct Signal {
  std::string strat, sym;
  Side side;
  double qty, limit_px;
  int64_t ts_ns;
};
enum class OrdType { Market, Limit, Cancel, Modify };
struct OrderReq {
  std::string id, sym;
  Side side;
  OrdType type;
  double qty, px;
  int64_t ts_ns;
};
enum class OrdStatus { Accepted, Rejected, Filled, Partial, Canceled };
struct OrderResp {
  std::string id;
  OrdStatus st;
  double filled_qty;
  double avg_px;
  const char *reason;
  int64_t ts_ns;
  std::string err;
};
