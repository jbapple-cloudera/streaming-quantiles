#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <utility>

using namespace std;

template <typename T, int32_t CAPACITY>
struct Reservoir {
 private:
  mutable array<T, CAPACITY> data_;
  uint64_t size_;

 public:
  explicit Reservoir() : data_(), size_(0) {}

  const uint64_t& size = size_;

  template <typename Random>
  void Insert(Random* rgen, const T& key, uint8_t) {
    if (size_ < CAPACITY) {
      data_[size_] = key;
      ++size_;
      return;
    }
    auto dist = uniform_int_distribution<uint32_t>(0, size_);
    const auto place = dist(*rgen);
    if (place < CAPACITY) data_[place] = key;
    ++size_;
  }

 public:
  T Percentile(double p) const {
    assert(0 <= p && p <= 1);
    const int32_t len = min(static_cast<uint64_t>(CAPACITY), size_);
    p -= 0.5 / len;
    const uint32_t place = max(0, min(len - 1, static_cast<int32_t>(p * len)));
    nth_element(data_.begin(), &data_[place], data_.end());
    return data_[place];
  }

  template <typename Random>
  void Merge(Random* rgen, const Reservoir& that);
};
