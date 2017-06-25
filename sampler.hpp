#pragma once

// Reservoir sampling with a sample size of 1; used in the KLL sketch.

#include <cassert>
#include <cstdint>
#include <random>
#include <iostream>
#include <fstream>

namespace std {
template <>
class is_integral<unsigned __int128> {
 public:
  static constexpr bool value = true;
};
}

namespace sampler {

// The simplest reservoir sampling
template <typename N>
class Simple {
  N count = 0;

 public:
  static constexpr auto NAME() { return "sampler::Simple"; }

  // Returns true if an item is kept at this step
  template <typename Urng>
  bool Step(Urng* urng) {
    // TODO: libstdc++'s uniform_int_distribution throws away bits. Make this a member
    // variable and keep leftover data to use in next call.
    auto dist = ::std::uniform_int_distribution<N>(0, count);
    ++count;
    return 0 == dist(*urng);
  }
};

// Faster sampling using "Reservoir-sampling algorithms of time complexity
// O(n(1+log(N/n)))" by Kim-Hung Li.
template <typename N>
class Li {
  // TODO: make work for __float128
  long double lowest_hash = 1.0;
  N skip = 0;
  template <typename Urng>
  static long double UniformUpTo(long double f, Urng* urng) {
    return std::uniform_real_distribution<long double>(
        std::nextafter(0.0L, 1.0L), std::nextafter(f, 0.0L))(*urng);
  }
  template <typename Urng>
  static N Countdown(long double f, Urng* urng) {
    thread_local std::exponential_distribution<long double> expo;
    return std::floor(-expo(*urng) / std::log(1.0L - f));
  }

 public:
  static constexpr auto NAME() { return "sampler::Li    "; }
  template <typename Urng>
  bool Step(Urng* g) {
    if (skip) {
      --skip;
      return false;
    }
    lowest_hash = UniformUpTo(lowest_hash, g);
    skip = Countdown(lowest_hash, g);
    return true;
  }
};

template <typename N>
struct Ratio {
  N num = 0, den = 1;
};

struct Order {
  enum Ordering { LT, GT, EQ };

  template <typename N>
  static Ordering Compare(const std::vector<bool>& r, const Ratio<N> p) {
    // TODO: use type larger than bool for faster Compare
    N num = p.num;
    for (const bool v : r) {
      if (v) {
        if (num < p.den / 2 + (p.den & 1)) return GT;
        num = num - (p.den - num);
      } else {
        if (num >= p.den / 2 + (p.den & 1)) return LT;
        num = 2 * num;
      }
    }
    return num ? LT : EQ;
  }
};

bool Increment(std::vector<bool>& r) {
  for (uint64_t i = r.size() - 1; i < r.size(); --i) {
    r[i] = !r[i];
    if (r[i]) return true;
  }
  return false;
}

bool Decrement(std::vector<bool>& r) {
  for (uint64_t i = r.size() - 1; i < r.size(); --i) {
    r[i] = !r[i];
    if (!r[i]) return true;
  }
  return false;
}

enum struct Direction { LHS, RHS };

template <typename CDF, Direction D, typename N>
N Invert(const std::vector<bool>& r, N count, N lo, N hi) {
  const auto cdf = [count](N s) { return CDF::F(count, s); };
  assert(Order::EQ == Order::Compare(std::vector<bool>(), cdf(0)));
  assert(hi > lo);
  for (N incr = 1; (incr > 0) and (N(lo + incr) > lo) and (N(lo + incr) < hi);
       incr = incr * 2) {
    switch (Order::Compare(r, cdf(lo + incr))) {
      case Order::LT: hi = lo + incr; goto done;
      case Order::EQ: if (D == Direction::RHS) return lo + incr;
                      lo = lo + incr; goto done;
      case Order::GT: lo = lo + incr;
    }
  }
done:
  while (hi - lo > 1) {
    assert(Order::LT != Order::Compare(r, cdf(lo)));
    assert(hi + count == std::numeric_limits<N>::max()
        || Order::GT != Order::Compare(r, cdf(hi)));
    const N mid = lo + (hi - lo)/2;
    switch (Order::Compare(r, cdf(mid))) {
      case Order::LT: hi = mid; break;
      case Order::GT: lo = mid; break;
      case Order::EQ: if (D == Direction::LHS) lo = mid; else return mid;
    }
  }
  assert (hi - lo == 1);
  assert(Order::LT != Order::Compare(r, cdf(lo)));
  assert(hi + count == std::numeric_limits<N>::max()
      || Order::GT != Order::Compare(r, cdf(hi)));
  return hi;
}

template <typename U>
class OneBit {
 protected:
  U buffer = 0;
  U buffer_size = 0;

 public:
  template <typename G>
  bool operator()(G& g) {
    if (0 == buffer_size) {
      buffer = g();
      buffer_size = std::min(sizeof(buffer), sizeof(g()));
    }
    const bool result = 1 == (buffer % 2);
    buffer = buffer / 2;
    --buffer_size;
    return result;
  }
};

template <typename CDF, typename N, typename R>
N Sample(R* urng, N count) {
  if (0 == count) return 0;
  N lo = 0, hi = std::numeric_limits<N>::max() - count;
  thread_local OneBit<typename R::result_type> rng;
  for (std::vector<bool> r(1, rng(*urng));; r.push_back(rng(*urng))) {
    lo = Invert<CDF, Direction::LHS>(r, count, lo, hi) - 1;
    assert(lo + 1 != 0);
    if (lo + 1 >= hi) return hi - 1;
    if (!Increment(r)) {
      const bool d = Decrement(r);
      assert(!d);
      continue;
    }
    hi = Invert<CDF, Direction::RHS>(r, count, lo, hi);
    assert(hi > lo);
    if (hi - lo == 1) return hi - 1;
    const bool d = Decrement(r);
    assert(d);
  }
}

template <typename N>
struct VitterCDF {
  static Ratio<N> F(N count, N s) { return {s, N(s + count)}; }
};

template <typename N>
class Vitter {
  N count = 0, skip = 0;

 public:
  static constexpr auto NAME() { return "sampler::Vitter"; }
  template <typename G>
  bool Step(G* g) {
    ++count;
    if (skip) {
      --skip;
      return false;
    }
    skip = Sample<VitterCDF<N>>(g, count);
    return true;
  }
};

long double AsFloating(const std::vector<bool>& r) {
  long double result = 0.0;
  for (auto i = r.size() - 1; i < r.size(); --i) {
    result = result / 2;
    result += r[i] ? 1 : 0;
  }
  return result / 2;
}

template <typename N>
long double AsFloating(const Ratio<N>& r) {
  return static_cast<long double>(r.num) / r.den;
}


template <typename ResultType>
struct PromptPrng {
  using result_type = ResultType;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return static_cast<result_type>(~static_cast<result_type>(0));
  }
  result_type operator()() const {
    std::uintmax_t result;
    std::cin >> result;
    return result;
  }
};

template <typename ResultType>
struct DevUrandom {
  using result_type = ResultType;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
  std::ifstream s;
  explicit DevUrandom() : s("/dev/urandom", std::ios::binary) { assert(s.is_open()); }
  result_type operator()() {
    assert(s.is_open());
    result_type result = 0;
    s.read(reinterpret_cast<char*>(&result), sizeof(result));
    return result;
  }
};

template <typename ResultType>
struct DevUrandomBuffer {
  using result_type = ResultType;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
  std::ifstream s;
  size_t buf_len = 0;
  std::vector<result_type> buffer;
  explicit DevUrandomBuffer(size_t size)
    : s("/dev/urandom", std::ios::binary), buffer(size) {
    assert(s.is_open());
    s.read(reinterpret_cast<char *>(buffer.data()), sizeof(result_type) * buffer.size());
    buf_len = buffer.size();
  }
  result_type operator()() {
    if (buf_len == 0) {
      assert(s.is_open());
      s.read(reinterpret_cast<char*>(buffer.data()), sizeof(result_type) * buffer.size());
      buf_len = buffer.size();
    }
    --buf_len;
    return buffer[buf_len];
  }
};

template <typename ResultType>
struct ExplodingPrng {
  using result_type = ResultType;
  result_type r;
  ExplodingPrng(size_t r) : r(r) {}
  size_t count = 100;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return static_cast<result_type>(~static_cast<result_type>(0));
  }
  result_type operator()() {
    if (0 == count) throw r;
    --count;
    return r;
  }
};
}
