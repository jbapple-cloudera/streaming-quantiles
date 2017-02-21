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

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if not __has_builtin(__builtin_unreachable)
#define __builtin_unreachable()
#endif

template<typename Clock, typename F>
auto PrintTimerWithClock(const F& f) {
  const auto start = Clock::now();
  const auto result = f();
  const auto finish = Clock::now();
  const auto delta = finish - start;
  std::cout.imbue(std::locale(""));
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(delta).count()
            << " milliseconds" << std::endl;
  return result;
}

template<typename F>
auto PrintTimer(const F& f) {
  if (std::chrono::high_resolution_clock::is_steady) {
    return PrintTimerWithClock<std::chrono::high_resolution_clock>(f);
  } else {
    return PrintTimerWithClock<std::chrono::steady_clock>(f);
  }
}

template<typename T>
auto GroundTruth(const std::vector<T>& keys) {
  std::unordered_map<T, std::pair<double, double>> index;
  const double total = keys.size();
  std::unordered_map<T, double> accum;
  for (const auto& key : keys) ++accum[key];

  std::vector<std::pair<T, double>> sorted(accum.begin(), accum.end());
  sort(sorted.begin(), sorted.end());
  double running = 0;
  for (const auto& v : sorted) {
    auto next = running + v.second;
    assert(next <= total);
    index[v.first] = {running / total, next / total};
    running = next;
  }
  std::cout.imbue(std::locale(""));
  std::cout << "TOTAL KEYS: " << static_cast<uintmax_t>(total) << std::endl
            << "UNIQUE KEYS: " << index.size() << std::endl;
  return index;
}

auto GroundTruth(const std::string& filename) {
  std::vector<std::string> keys;
  std::string word;
  std::ifstream dict(filename);
  while (dict >> word) {
    keys.push_back(word);
  }
  return GroundTruth(keys);
}

template <typename Random, typename Sketch>
Sketch ComputeSketch(const std::string& filename) {
  Sketch sketch;
  std::ifstream dict(filename);
  Random r;
  std::string word;
  std::cout << "COMPUTING SKETCH" << std::endl;
  while (dict >> word) sketch.Insert(&r, word, 0);
  std::cout << "SKETCH COMPUTED" << std::endl;
  return sketch;
}

template<typename Random, typename... Sketches>
void InteractiveTest(const std::string& filename) {
  std::cout.imbue(std::locale(""));
  std::cout << filename << std::endl;
  const auto index = PrintTimer([&] { return GroundTruth(filename); });
  const auto sketches = std::make_tuple(ComputeSketch<Random, Sketches>(filename)...);
  double p;
  std::cout.precision(4);
  std::array<std::pair<double, double>, sizeof...(Sketches)> truths;
  while (std::cin >> p) {
    const std::array<std::string, sizeof...(Sketches)> results = {
        std::get<Sketches>(sketches).Percentile(p / 100)...};
    std::transform(results.begin(), results.end(), truths.begin(),
        [&](const std::string& result) { return index.find(result)->second; });
    for (int i = 0; i < sizeof...(Sketches); ++i) {
      std::cout << 100 * truths[i].first << ' ' << 100 * truths[i].second;
      std::cout << ' ' << results[i] << std::endl;
    }
  }
}


template<typename Random, typename Sketch>
void Benchmark(const std::string& filename) {
  std::ifstream file(filename);
  Sketch sketch;
  std::string word;
  Random r;
  //uint64_t count = 0;
  while (file >> word) {
    sketch.Insert(&r, word, 0);
    //++count;
    //assert(count == sketch.InferredSize());
  }
  //std::cout << count << ' ' << sketch.InferredSize() << std::endl;
}

template<typename Random, typename Sketch>
std::string Middle(const std::string& filename) {
  std::ifstream file(filename);
  Sketch sketch;
  std::string word;
  Random r;
  while (file >> word) sketch.Insert(&r, word, 0);
  return sketch.Percentile(0.5);
}

template <typename Random, typename Sketch>
std::string Middle(const std::vector<std::string>& keys) {
  Sketch sketch;
  Random r;
  for (const auto& key : keys) sketch.Insert(&r, key, 0);
  const auto result = sketch.Percentile(0.5);
  //std::cout << result << ' ';
  return result;
}

double Error(const std::pair<double,double>& range) {
  const auto //
      lo = std::max(0.0, range.first - 0.5), //
      hi = std::max(0.0, 0.5 - range.second);
  assert(lo >= 0);
  assert(hi >= 0);
  return std::max(lo, hi);
}

template <typename Random, typename... Sketches>
void Quality(const std::string& filename) {
  std::vector<std::string> keys;
  {
    std::string word;
    std::ifstream file(filename);
    while (file >> word) keys.push_back(word);
  }

  const auto index = PrintTimer([&] { return GroundTruth(keys); });
  uint64_t count = 0;
  std::array<double, sizeof...(Sketches)> sum_abs_error{}, max_error{}, sum_sqr_error{};
  std::array<std::string, sizeof...(Sketches)> estimates;
  std::array<std::pair<double, double>, sizeof...(Sketches)> truths;
  std::array<double, sizeof...(Sketches)> errors;
  for (uint64_t count = 1; true; ++count) {
    estimates = {Middle<Random, Sketches>(keys)...};
    //std::cout << std::endl;
    std::transform(estimates.begin(), estimates.end(), errors.begin(), [&](const auto& s) {
      return Error(index.find(s)->second);
    });

    std::transform(errors.begin(), errors.end(), sum_abs_error.begin(),
              sum_abs_error.begin(), std::plus<>());
    /*
    std::transform(errors.begin(), errors.end(), sum_sqr_error.begin(), sum_sqr_error.begin(),
        [](double err, double sum) { return err * err + sum; });
    */
    std::transform(errors.begin(), errors.end(), max_error.begin(), max_error.begin(),
        [](double err, double big) { return std::max(err, big); });
    if (static_cast<uint64_t>(sqrt(count)) * static_cast<uint64_t>(sqrt(count))
        == count) {
      std::cout.precision(4);
      std::cout << std::fixed;
      std::cout << std::setw(6) << count;
      for (auto v : sum_abs_error) std::cout << std::setw(8) << 100 * v / count;
      std::cout << std::endl << std::setw(6) << "max";
      for (auto v : max_error) std::cout << std::setw(8) << 100 * v;
      // for (auto v : sum_sqr_error) std::cout << 100 * sqrt(v / count) << ' ';
      std::cout << std::endl;
    }
  }

}

template <typename T>
T FindPercentile(std::vector<std::pair<T, uint64_t>> keys, double p) {
  assert(0 <= p && p <= 1);
  sort(keys.begin(), keys.end());

  double target = 0;
  for (const auto& v : keys) target += v.second;
  target *= p;

  uint64_t seen = 0;
  //std::cout << keys.size() << ' ' << p << std::endl;
  for (const auto& v : keys) {
    seen += v.second;
    //std::cout << v.first << ' ' << v.second << std::endl;
    if (seen >= target) return v.first;
  }
  __builtin_unreachable();
}
