#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

#include "prometheus/detail/ckms_quantiles.h"  // IWYU pragma: export
#include "prometheus/detail/core_export.h"
#include <mutex>

namespace prometheus {
namespace detail {

class PROMETHEUS_CPP_CORE_EXPORT TimeWindowMax {
 public:
  using Clock = std::chrono::steady_clock;

  TimeWindowMax(Clock::duration expiry, int buffer_length);

  void Record(int64_t value);
  int64_t Get();

 private:
  std::atomic_int64_t& Rotate();

 private:
  Clock::duration expiry_;
  int buffer_length_;
  std::vector<std::atomic_int64_t> buffer_;
  std::size_t current_bucket_{0};
  Clock::time_point last_rotation_;
  std::mutex mutex_;
};

}  // namespace detail
}  // namespace prometheus
