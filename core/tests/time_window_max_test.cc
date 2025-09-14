#include "prometheus/detail/time_window_max.h"

#include <gtest/gtest.h>

#include <thread>

namespace {
void sleep_more(std::chrono::steady_clock::duration ms) {
  std::this_thread::sleep_for(ms + std::chrono::milliseconds(10));
}
}  // namespace

namespace prometheus {
namespace detail {

using Clock = TimeWindowMax::Clock;

TEST(TimeWindowMaxTest, basic) {
  int buffer_length = 3;
  Clock::duration expiry = std::chrono::milliseconds(100);
  TimeWindowMax twm{expiry, buffer_length};
  twm.Record(100);

  for (int i = 0; i < buffer_length; ++i) {
    EXPECT_EQ(100, twm.Get());
    sleep_more(expiry);
  }

  EXPECT_EQ(0, twm.Get());
}

TEST(TimeWindowMaxTest, rotate) {
  int buffer_length = 3;
  Clock::duration expiry = std::chrono::milliseconds(100);
  TimeWindowMax twm{expiry, buffer_length};
  twm.Record(32);
  EXPECT_EQ(32, twm.Get());  // 32 | 0 | 0

  sleep_more(expiry * buffer_length);
  EXPECT_EQ(0, twm.Get());  // 0  | 0 | 0

  twm.Record(666);
  EXPECT_EQ(666, twm.Get());  // 666 | 0 | 0

  sleep_more(expiry);
  twm.Record(500);
  EXPECT_EQ(666, twm.Get());  // 500 | 666 |

  sleep_more(expiry);
  twm.Record(10500);
  EXPECT_EQ(10500, twm.Get());  // 500 | 666 | 10500
}

}  // namespace detail
}  // namespace prometheus
