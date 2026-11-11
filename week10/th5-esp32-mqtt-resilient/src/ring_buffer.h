#pragma once
#include <stddef.h>
#include <stdint.h>

// Fixed-capacity FIFO. When full, push() discards the oldest element so the
// newest data always survives a long outage. No heap allocation.
template <typename T, size_t N>
class RingBuffer {
  static_assert(N > 0, "capacity must be greater than zero");

 public:
  bool empty() const { return count_ == 0; }
  size_t size() const { return count_; }
  static constexpr size_t capacity() { return N; }
  uint32_t dropped() const { return dropped_; }

  void push(const T& item) {
    if (count_ == N) {
      head_ = (head_ + 1) % N;
      --count_;
      ++dropped_;
    }
    data_[(head_ + count_) % N] = item;
    ++count_;
  }

  // Oldest element. Only valid when !empty().
  const T& front() const { return data_[head_]; }

  void pop() {
    if (count_ == 0) return;
    head_ = (head_ + 1) % N;
    --count_;
  }

  void clear() {
    head_ = 0;
    count_ = 0;
  }

 private:
  T data_[N];
  size_t head_ = 0;
  size_t count_ = 0;
  uint32_t dropped_ = 0;
};
