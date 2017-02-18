#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

#include "reservoir.hpp"
#include "utility.hpp"

template<typename T, uint32_t CAPACITY>
struct Kll {
private:
  vector<vector<T>> data_;
  deque<uint32_t> size_limits_;
  uint64_t size_;
  static uint32_t Round(uint32_t x) { return 2 * (x / 2); }

 public:
  explicit Kll() : data_(), size_limits_(), size_(0) {
    size_limits_.push_back(Round(CAPACITY / 3));
    data_.emplace_back();
  }

  const uint64_t& size = size_;

  template <typename Random>
  void Insert(Random* rgen, const T& key, uint8_t level) {
    assert (level <= data_.size());
    assert (size_limits_.size() == data_.size());
    ++size_;
    if (level >= data_.size()) {
      data_.push_back(vector<T>());
      size_limits_.push_front(Round(size_limits_[0] * 2 / 3));
    }
    if (data_[level].size() >= size_limits_[level]) {
      if (data_[level].size() > 2) {
        sort(data_[level].begin(), data_[level].end());
      }
      for (uint32_t i = rgen->Bool(); i < data_[level].size(); i += 2) {
        Insert(rgen, data_[level][i], level+1);
      }
      data_[level].clear();
    }
    data_[level].push_back(key);
  }

 private:
  vector<pair<T, uint64_t>> Flatten() const {
    vector<pair<T, uint64_t>> result;
    uint64_t weight = 1;
    for (const auto& d : data_) {
      weight *= 2;
      for (const auto& v : d) {
        result.push_back({v, weight});
      }
    }
    return result;
  }

 public:
  T Percentile(double p) const {
    assert(0 <= p && p <= 1);
    auto keys = Flatten();
    sort(keys.begin(), keys.end());

    double target = 0;
    for (const auto& v : keys) target += v.second;
    target *= p;

    uint64_t seen = 0;
    for (const auto& v : keys) {
       seen += v.second;
       if (seen >= target) return v.first;
    }
    __builtin_unreachable();
  }

  template <typename Random>
  void Merge(Random* rgen, const Kll& that) {
    uint8_t level = 0;
    for (const auto& row : that.data_) {
      for (const auto& key : row) {
        Insert(rgen, key, level);
      }
      ++level;
    }
    size_ += that.size_;
  }
};

int main(int argc, char ** argv) {
  assert (argc == 2);
  // Quality<UrandomBool, Kll<string, 1000>, Reservoir<string, 20000>>(argv[1]);
  // InteractiveTest<UrandomBool, Reservoir<string, 1000>>(argv[1]);
  PrintTimer([&] { Benchmark<UrandomBool, Reservoir<string, 20000>>(argv[1]); return 0; });
  PrintTimer([&] { Benchmark<UrandomBool, Kll<string, 1000>>(argv[1]); return 0; });
  PrintTimer([&] { Benchmark<UrandomBool, Reservoir<string, 20000>>(argv[1]); return 0; });
}
