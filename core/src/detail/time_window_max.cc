#include "prometheus/detail/time_window_max.h"

#include <iostream>

namespace {
struct RotateGuard {
  RotateGuard(std::atomic_bool& rotate) : rotate_(rotate) {}
  ~RotateGuard() { rotate_.store(false); }

 private:
  std::atomic_bool& rotate_;
};
}  // namespace

namespace prometheus {
namespace detail {

TimeWindowMax::TimeWindowMax(Clock::duration expiry, int buffer_length)
    : expiry_{expiry}, buffer_length_{buffer_length}, buffer_(buffer_length) {
  last_rotation_ = Clock::now();
  rotate_.store(false);
}

void TimeWindowMax::Record(int64_t value) {
  Rotate<true>();
  for (auto& v : buffer_) {
    int64_t current = v.load();
    while (value > current && !v.compare_exchange_weak(current, value)) {
      // intentionally empty block
    }
  }
}

int64_t TimeWindowMax::Get() {
  std::lock_guard<std::mutex> lock(mutex_);
  Rotate<false>();
  return buffer_[current_bucket_].load();
}

template<bool with_lock>
void TimeWindowMax::Rotate() {
  auto now = Clock::now();
  auto time_since_last_rotation = now - last_rotation_;

  if (time_since_last_rotation < expiry_) {
    return;
  }

  bool expected = false;
  if (!rotate_.compare_exchange_strong(expected, true)) {
    return;
  }

  auto rotate = [&]() {
    RotateGuard guard{rotate_};
    if (time_since_last_rotation > buffer_length_ * expiry_) {
      for (auto& bucket : buffer_) {
        bucket.store(0);
      }
      current_bucket_ = 0;
      last_rotation_ = now - time_since_last_rotation % expiry_;
      return;
    }

    int iter = 0;
    do {
      buffer_[current_bucket_].store(0);
      current_bucket_++;
      if (current_bucket_ >= buffer_length_) {
        current_bucket_ = 0;
      }
      time_since_last_rotation -= expiry_;
      last_rotation_ += expiry_;
    } while (time_since_last_rotation > expiry_ && ++iter < buffer_length_);
  };

  if (with_lock) {
    std::lock_guard<std::mutex> lock(mutex_);
    rotate();
  } else {
    rotate();
  }
}

}  // namespace detail
}  // namespace prometheus
