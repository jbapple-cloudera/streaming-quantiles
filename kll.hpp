#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "utility.hpp"

template<typename T, uint32_t CAPACITY>
struct Kll {
private:
  std::vector<std::vector<T>> data_;
  std::deque<uint32_t> size_limits_;
  uint64_t size_;
  static uint32_t Round(uint32_t x) { return 2 * (x / 2); }

 public:
  explicit Kll() : data_(), size_limits_(), size_(0) {
    size_limits_.push_back(Round(CAPACITY / 3));
    data_.emplace_back();
  }

  const uint64_t& size = size_;

  void PrintMetaData() {
    return;
    for (const auto & level : data_) {
      std::cout << " ";
      std::cout << std::setw(4) << std::right << level.size();
    }
    std::cout << std::endl;
  }

  uint64_t InferredSize() const {
    uint64_t result = 0, weight = 1;
    for (const auto& level : data_) {
      result += weight * level.size();
      weight *= 2;
    }
    return result;
  }

  template <typename Random>
  void Insert(Random* rgen, const T& key, uint16_t level) {
    assert (level <= data_.size());
    assert (size_limits_.size() == data_.size());
    ++size_;
    if (level >= data_.size()) {
      data_.push_back(std::vector<T>());
      size_limits_.push_front(Round(size_limits_[0] * 2 / 3));
    }
    if (data_[level].size() >= size_limits_[level]) {
      PrintMetaData();
      if (data_[level].size() > 2) {
        std::sort(data_[level].begin(), data_[level].end());
      }
      std::uniform_int_distribution<uint32_t> dist(0,1);
      for (uint32_t i = dist(*rgen); i < data_[level].size(); i += 2) {
        Insert(rgen, data_[level][i], level+1);
      }
      data_[level].clear();
    }
    data_[level].push_back(key);
  }

 public:
  Cdf<T> GetCdf() const {
    std::vector<std::pair<T, int64_t>> result;
    int64_t weight = 1;
    for (const auto& d : data_) {
      weight *= 2;
      for (const auto& v : d) result.push_back({v, weight});
    }
    std::sort(result.begin(), result.end());
    return Cdf<T>(result);
  }

 public:
  // T Percentile(double p) const {
  //   return FindPercentile(Flatten(), p);
  // }

  template <typename Random>
  void Merge(Random* rgen, const Kll& that) {
    uint16_t level = 0;
    for (const auto& row : that.data_) {
      for (const auto& key : row) {
        Insert(rgen, key, level);
      }
      ++level;
    }
    size_ += that.size_;
  }
};
