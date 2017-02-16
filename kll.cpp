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

struct UrandomBool {
  random_device dev_;
  unsigned cache_;
  int cache_size_;
  explicit UrandomBool() : dev_(), cache_(0), cache_size_(0) {}
  bool Bool() {
    if (cache_size_ == 0) {
      cache_ = dev_();
      cache_size_ = CHAR_BIT * sizeof(cache_);
    }
    const bool result = cache_ & 1;
    cache_ >>= 1;
    --cache_size_;
    return result;
  }
};

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
      sort(data_[level].begin(), data_[level].end());
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
  }
};

int main(int argc, char ** argv) {
  assert (argc == 2);
  string word;
  UrandomBool r;
  unordered_map<string, pair<double, double>> index;
  double total = 0;
  {

    ifstream dict(argv[1]);
    unordered_map<string, double> accum;
    while (dict >> word) {
      ++accum[word];
      ++total;
    }
    vector<pair<string, double>> sorted(accum.begin(), accum.end());
    sort(sorted.begin(), sorted.end());
    double running = 0;
    for (const auto& v: sorted) {
      auto next = running + v.second;
      assert (next <= total);
      index[v.first] = {running / total, next / total};
      running = next;
    }
    cout << "TOTAL: " << total << endl;
  }
  Kll<string, 200> kll;
  ifstream dict(argv[1]);
  while (dict >> word) kll.Insert(&r, word, 0);
  const auto result = kll.Percentile(0.5);
  cout << result << ' ' << index[result].first << ' ' << index[result].second << endl;
}
