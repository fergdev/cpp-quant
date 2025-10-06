#include "bus.hpp"
#include "risk.hpp"
#include "types.hpp"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <chrono>
#include <future>
#include <gtest/gtest.h>

namespace asio = boost::asio;

TEST(RiskStage, PassesValidSignalAsMarketOrder) {
  asio::io_context ioc;

  AsioChan<Signal> sig_in{ioc, 1};
  AsioChan<OrderReq> ord_out{ioc, 1};

  Risk risk{sig_in, ord_out};
  risk.start();

  Signal s{};
  s.strat = "sma20";
  s.sym = "BTCUSDT";
  s.side = Side::Buy;
  s.qty = 0.01;
  s.ts_ns = 123456789;

  std::promise<OrderReq> got_order;

  asio::co_spawn(
      ioc,
      [&]() -> asio::awaitable<void> {
        co_await sig_in.async_send({}, s, asio::use_awaitable);

        OrderReq r = co_await ord_out.async_receive(asio::use_awaitable);
        got_order.set_value(r);
        ioc.stop();
        co_return;
      },
      asio::detached);

  std::thread runner([&] { ioc.run(); });

  auto fut = got_order.get_future();
  ASSERT_EQ(fut.wait_for(std::chrono::seconds(2)), std::future_status::ready)
      << "Timed out waiting for Risk output";

  const OrderReq r = fut.get();
  runner.join();

  EXPECT_EQ(r.sym, "BTCUSDT");
  EXPECT_EQ(r.qty, 0.01);
  EXPECT_EQ(r.type, OrdType::Market);
  EXPECT_EQ(r.ts_ns, 123456789);
  EXPECT_FALSE(r.id.empty());
}
