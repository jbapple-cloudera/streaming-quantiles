#pragma once

/// A version of the KLL mergable streaming quantiles.
///
/// The class SampledKll<T, N> stores at most N values of type T in a sketch that can
/// answer quantile questions such as "how many stream values are greater than v". Two
/// factors are used when discussing the accuracy of the benchmarks:
///
///  1. ε, the approximation ratio. The returned answer may be off by 100*ε percentile.
///  2. δ, the failure rate. With probability δ, the answer returned is off by more than
///     ε.
///
/// The sketch in this file uses the KLL datastructure with the sampler but without the
/// MRL or GK sketch on top. Its space usage (N) is -\sqrt{ln δ}/ε to answer a single
/// quantile or -\sqrt{ln δε}/ε to answer all quantile queries correctly.
///
/// This sketch supports three operations: Insert(T), CDF(), and Merge(SampledKll).

#include <algorithm>
#include <bitset>
#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include "utility.hpp"

// template <int32_t CAPACITY>
// void PrintKllArray() {
//   const auto f = KllArrayDetails::KllArray<CAPACITY>();
//   for (auto v : f) std::cout << v << std::endl;
//   std::cout << "--\n";
// }

template <typename T, int32_t CAPACITY>
struct SampledKll {
 private:
  class KllArrayDetails {
    template <typename Int>
    static constexpr Int Round(Int x) {
      return (4 < (2 * (x / 2))) ? (2 * (x / 2)) : 4;
    }

    static constexpr int16_t KllHeight(int32_t capacity) {
      return (capacity < 4) ? 0 : (1 + KllHeight(capacity - Round(capacity / 3)));
    }

    // static_assert(KllHeight(numeric_limits<int32_t>::max(), 0) == 50,
    //  "KllHeight different than expected");

    static constexpr int16_t KllHeightNth(int32_t capacity, int16_t n) {
      return (n + 1 == KllHeight(capacity)) ?
          (capacity - Round(capacity / 3)) :
          KllHeightNth(capacity - Round(capacity / 3), n);
    }

    template <bool TIGHT_FIT>
    using KllArrayType =
        std::array<int32_t, KllArrayDetails::KllHeight(CAPACITY - TIGHT_FIT) + 1>;

    template <bool TIGHT_FIT, int16_t... INDEXES>
    static constexpr KllArrayType<TIGHT_FIT> KllArrayMake(
        std::integer_sequence<int16_t, INDEXES...>) {
      return std::array<int32_t, KllHeight(CAPACITY - TIGHT_FIT) + 1>{
          TIGHT_FIT + KllHeightNth(CAPACITY - 1, INDEXES)..., CAPACITY};
    }

    static constexpr int16_t KllTightFit(int32_t capacity) {
      return (capacity < 4) ? (0 == capacity) :
                              KllTightFit(capacity - Round(capacity / 3));
    }

   public:
    static constexpr auto KllArray() {
      return KllArrayMake<KllTightFit(CAPACITY)>(std::make_integer_sequence<int16_t,
          KllHeight(CAPACITY - KllTightFit(CAPACITY))>());
    }
  };

  using KllArrayType = decltype(KllArrayDetails::KllArray());

  static constexpr KllArrayType LEVEL_START = KllArrayDetails::KllArray();
  std::array<T, CAPACITY> data_{};
  std::array<int32_t, LEVEL_START.size() - 1> level_sizes_{};
  int64_t sample_weight_ = 0;
  std::bitset<LEVEL_START.size() - 1> heavies_ = 0;
  int16_t sample_height_ = 1 - level_sizes_.size();

 public:
  Cdf<T> GetCdf() const {
    std::vector<std::pair<T, double>> raw;
    if (sample_weight_) raw.push_back({data_[0], sample_weight_});
    int64_t weight = 1ll << std::max(0, +sample_height_);
    for (int16_t level = std::max(0, -sample_height_); level < level_sizes_.size();
         ++level) {
      for (int32_t i = 0; i < level_sizes_[level]; ++i) {
        raw.push_back({data_[LEVEL_START[level] + i], weight});
      }
      weight *= 2;
    }
    std::sort(raw.begin(), raw.end());
    return Cdf<T>(raw);
  }

 private:
  template <typename Random>
  void Compress(Random* rgen, int16_t level, int32_t len) {
    // std::cout << "Compress level: " << level << std::endl;
    T* const keys = &data_[LEVEL_START[level]];
    std::sort(keys, keys + len);
    std::uniform_int_distribution<int32_t> dist(0, 1);
    for (int32_t i = dist(*rgen); i < len; i += 2) {
      keys[i / 2] = keys[i];
    }
    heavies_[level] = true;
    level_sizes_[level] = len / 2;
  }

  template <typename Random>
  void ShuffleDown(Random* rgen) {
    // std::cout << "ShuffleDown" << std::endl;

    using std::swap;
    std::array<T, LEVEL_START[1] - LEVEL_START[0]> purgatory{};
    int32_t purgatory_size = 0;
    if (!heavies_[0]) {
      std::copy(&data_[LEVEL_START[0]], &data_[LEVEL_START[0] + level_sizes_[0]],
          &purgatory[0]);
      swap(purgatory_size, level_sizes_[0]);
    }
    for (int16_t level = 1; level < level_sizes_.size(); ++level) {
      if (heavies_[level]) continue;
      bool copied_up = false;
      while (level_sizes_[level] > 0) {
        if (level_sizes_[level - 1] >= LEVEL_START[level] - LEVEL_START[level - 1]) {
          Compress(rgen, level - 1, level_sizes_[level - 1]);
          std::copy(&data_[LEVEL_START[level - 1]],
              &data_[LEVEL_START[level - 1] + level_sizes_[level - 1]],
              &data_[LEVEL_START[level + 1]
                  - (LEVEL_START[level] - LEVEL_START[level - 1]) / 2]);
          copied_up = true;
          level_sizes_[level - 1] = 0;
        }
        data_[LEVEL_START[level - 1] + level_sizes_[level - 1]] =
            data_[LEVEL_START[level] + level_sizes_[level] - 1];
        ++level_sizes_[level - 1];
        --level_sizes_[level];
      }
      if (copied_up) {
        std::copy(&data_[LEVEL_START[level + 1]
                      - (LEVEL_START[level] - LEVEL_START[level - 1]) / 2],
            &data_[LEVEL_START[level + 1]], &data_[LEVEL_START[level]]);
        level_sizes_[level] = (LEVEL_START[level] - LEVEL_START[level - 1]) / 2;
      }
      heavies_[level] = true;
    }
    ++sample_height_;
    heavies_.reset();
    for (int16_t i = 0; i < purgatory_size; ++i) {
      Insert(rgen, purgatory[i], sample_height_ - 1);
    }
  }

  void PrintMetaData() const {
    return;
    std::cout << sample_weight_ << " " << sample_height_;
    for (int16_t i = 0; i < level_sizes_.size(); ++i) {
      std::cout << " ";
      if (heavies_[i]) std::cout << "H";
      std::cout << std::setw(4 - heavies_[i]) << std::right << level_sizes_[i] << "/"
                << std::setw(4) << std::left << LEVEL_START[i + 1] - LEVEL_START[i];
    }
    std::cout << std::endl;
  }

  // int64_t InferredSize() const {
  //   int64_t ans = sample_weight_;
  //   int64_t weight = (sample_height_ < 0) ? 1 : (1 << sample_height_);
  //   for (int i = std::max(0, -sample_height_); i < level_sizes_.size(); ++i) {
  //     ans += level_sizes_[i] * weight;
  //     weight *= 2;
  //   }
  //   return ans;
  // }

 public:
  template <typename Random>
  void Insert(Random* rgen, const T& key, int16_t key_height) {
    using std::swap;
    int16_t destination = key_height - sample_height_;
    while (destination >= 0
        && level_sizes_[destination]
            == LEVEL_START[destination + 1] - LEVEL_START[destination]) {
      // std::cout << "key_height: " << key_height << std::endl;
      PrintMetaData();
      Compress(rgen, destination, level_sizes_[destination]);
      PrintMetaData();
      if (destination == level_sizes_.size() - 1) {
        ShuffleDown(rgen);

      } else {
        while (level_sizes_[destination] > 0 && heavies_[destination]) {
          Insert(rgen, data_[LEVEL_START[destination] + level_sizes_[destination] - 1],
              key_height + 1);
          --level_sizes_[destination];
        }
        heavies_[destination] = false;
      }
      PrintMetaData();
      destination = key_height - sample_height_;
    }
    if (destination >= 0) {
      data_[LEVEL_START[destination] + level_sizes_[destination]] = key;
      level_sizes_[destination] += 1;
      return;
    }
    const int64_t limit_weight = 1ull << sample_height_;
    int64_t key_weight = 1ull << key_height;
    if (sample_weight_ + key_weight <= limit_weight) {
      std::uniform_int_distribution<int64_t> dist(0, sample_weight_ + key_weight - 1);
      if (dist(*rgen) < key_weight) {
        data_[0] = key;
      }
      sample_weight_ += key_weight;
      if (sample_weight_ == limit_weight) {
        sample_weight_ = 0;
        const auto temp_key = data_[0];
        Insert(rgen, temp_key, sample_height_);
      }
      return;
    }
    T mutable_key = key;
    if (sample_weight_ > key_weight) {
      swap(sample_weight_, key_weight);
      swap(data_[0], mutable_key);
    }
    std::uniform_int_distribution<int64_t> dist(0, limit_weight - 1);
    if (dist(*rgen) < key_weight) {
      Insert(rgen, mutable_key, sample_height_);
    }
  }
};

template <typename T, int32_t CAPACITY>
constexpr
    typename SampledKll<T, CAPACITY>::KllArrayType SampledKll<T, CAPACITY>::LEVEL_START;
