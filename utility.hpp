#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <limits>
#include <locale>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if not __has_builtin(__builtin_unreachable)
#define __builtin_unreachable()
#endif

struct UrandomBool {
  using result_type = unsigned;
  static constexpr result_type min() { return 0; };
  static constexpr result_type max() { return numeric_limits<result_type>::max(); }
  result_type operator()() { return dev_(); }
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

struct StupidBool {
  bool result = true;
  bool Bool() {
    result = !result;
    return result;
  }
};

template<typename Clock, typename F>
auto PrintTimerWithClock(const F& f) {
  const auto start = Clock::now();
  const auto result = f();
  const auto finish = Clock::now();
  const auto delta = finish - start;
  cout.imbue(std::locale(""));
  cout << chrono::duration_cast<chrono::milliseconds>(delta).count() << " milliseconds"
       << endl;
  return result;
}

template<typename F>
auto PrintTimer(const F& f) {
  if (chrono::high_resolution_clock::is_steady) {
    return PrintTimerWithClock<chrono::high_resolution_clock>(f);
  } else {
    return PrintTimerWithClock<chrono::steady_clock>(f);
  }
}

auto GroundTruth(const vector<string>& keys) {
  unordered_map<string, pair<double, double>> index;
  const double total = keys.size();
  unordered_map<string, double> accum;
  for (const auto& key : keys) ++accum[key];

  vector<pair<string, double>> sorted(accum.begin(), accum.end());
  sort(sorted.begin(), sorted.end());
  double running = 0;
  for (const auto& v : sorted) {
    auto next = running + v.second;
    assert(next <= total);
    index[v.first] = {running / total, next / total};
    running = next;
  }
  cout.imbue(std::locale(""));
  cout << "TOTAL KEYS: " << static_cast<uintmax_t>(total) << endl
       << "UNIQUE KEYS: " << index.size() << endl;
  return index;
}

auto GroundTruth(const string& filename) {
  vector<string> keys;
  string word;
  ifstream dict(filename);
  while (dict >> word) {
    keys.push_back(word);
  }
  return GroundTruth(keys);
}

template <typename Random, typename Sketch>
Sketch ComputeSketch(const string& filename) {
  Sketch sketch;
  ifstream dict(filename);
  Random r;
  string word;
  cout << "COMPUTING SKETCH" << endl;
  while (dict >> word) sketch.Insert(&r, word, 0);
  cout << "SKETCH COMPUTED" << endl;
  return sketch;
}

template<typename Random, typename Sketch>
void InteractiveTest(const string& filename) {
  cout.imbue(std::locale(""));
  cout << filename << endl;
  const auto index = PrintTimer([&] { return GroundTruth(filename); });
  const Sketch kll = PrintTimer([&] { return ComputeSketch<Random, Sketch>(filename); });
  double p;
  cout.precision(4);
  while (cin >> p) {
    const auto result = kll.Percentile(p / 100);
    const auto truth = index.find(result)->second;
    cout << 100 * truth.first << ' ' << 100 * truth.second;
    cout << ' ' << result << endl;
  }
}

template<typename Random, typename Sketch>
void Benchmark(const string& filename) {
  ifstream file(filename);
  Sketch sketch;
  string word;
  Random r;
  while (file >> word) sketch.Insert(&r, word, 0);
}

template<typename Random, typename Sketch>
string Middle(const string& filename) {
  ifstream file(filename);
  Sketch sketch;
  string word;
  Random r;
  while (file >> word) sketch.Insert(&r, word, 0);
  return sketch.Percentile(0.5);
}

template <typename Random, typename Sketch>
string Middle(const vector<string>& keys) {
  Sketch sketch;
  Random r;
  for (const auto& key : keys) sketch.Insert(&r, key, 0);
  return sketch.Percentile(0.5);
}

double Error(const pair<double,double>& range) {
  const auto //
      lo = max(0.0, range.first - 0.5), //
      hi = max(0.0, 0.5 - range.second);
  assert(lo >= 0);
  assert(hi >= 0);
  return max(lo, hi);
}

template <typename Random, typename... Sketches>
void Quality(const string& filename) {
  vector<string> keys;
  {
    string word;
    ifstream file(filename);
    while (file >> word) keys.push_back(word);
  }

  const auto index = PrintTimer([&] { return GroundTruth(keys); });
  uint64_t count = 0;
  array<double, sizeof...(Sketches)> sum_abs_error{}, max_error{}, sum_sqr_error{};
  array<string, sizeof...(Sketches)> estimates;
  array<pair<double, double>, sizeof...(Sketches)> truths;
  array<double, sizeof...(Sketches)> errors;
  for (uint64_t count = 1; true; ++count) {
    estimates = {Middle<Random, Sketches>(keys)...};
    transform(estimates.begin(), estimates.end(), errors.begin(), [&](const auto& s) {
      return Error(index.find(s)->second);
    });

    transform(errors.begin(), errors.end(), sum_abs_error.begin(),
              sum_abs_error.begin(), plus<>());
    /*
    transform(errors.begin(), errors.end(), sum_sqr_error.begin(), sum_sqr_error.begin(),
        [](double err, double sum) { return err * err + sum; });
    */
    transform(errors.begin(), errors.end(), max_error.begin(), max_error.begin(),
        [](double err, double big) { return max(err, big); });
    if (static_cast<uint64_t>(sqrt(count)) * static_cast<uint64_t>(sqrt(count))
        == count) {
      cout.precision(4);
      cout << fixed;
      cout << setw(6) << count;
      for (auto v : sum_abs_error) cout << setw(8) << 100 * v / count;
      cout << endl << setw(6) << "max";
      for (auto v : max_error) cout << setw(8) << 100 * v;
      // for (auto v : sum_sqr_error) cout << 100 * sqrt(v / count) << ' ';
      cout << endl;
    }
  }

}
