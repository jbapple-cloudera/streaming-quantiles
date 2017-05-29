#pragma once

// Reservoir sampling with a sample size of 1; used in the KLL sketch.

#include <cstdint>
#include <random>
#include <iostream>

namespace sampler {

// The simplest reservoir sampling
template <typename T>
struct simple {
  T payload;
  ::std::uint64_t count;
  simple(const T& payload) : payload(payload), count(1) {}
  template <typename G>
  void next(const T& x, G* g) {
    auto u = ::std::uniform_int_distribution<uint64_t>(0, count);
    ++count;
    if (0 == u(*g)) payload = x;
  }
};

// Faster sampling using "Reservoir-sampling algorithms of time complexity
// O(n(1+log⁡(N/n)))" by Kim-Hung Li.
//
// TODO: make work for FP == __float128
template <typename T, typename FP = long double>
struct exp {
  T payload;
  FP w;
  ::std::uint64_t s;
  template <typename G>
  static FP UniformUpTo(FP f, G* g) {
    return ::std::uniform_real_distribution<FP>(
        ::std::nextafter(static_cast<FP>(0.0), static_cast<FP>(1.0)),
        ::std::nextafter(f, static_cast<FP>(0.0)))(*g);
  }
  template <typename G>
  static ::std::uint64_t Countdown(FP f, G* g) {
    thread_local ::std::exponential_distribution<FP> expo;
    return ::std::floor(-expo(*g) / ::std::log(static_cast<FP>(1.0) - f));
  }
  template <typename G>
  exp(const T& payload, G* g)
    : payload(payload), w(UniformUpTo(1.0, g)), s(Countdown(w, g)) {}
  template <typename G>
  void next(const T& x, G* g) {
    if (s) {
      --s;
    } else {
      payload = x;
      w = UniformUpTo(w, g);
      s = Countdown(w, g);
      //::std::cout << w << '\t' << s << ::std::endl;
    }
  }
};
}
