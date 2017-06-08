#pragma once

// Reservoir sampling with a sample size of 1; used in the KLL sketch.

#include <cassert>
#include <cstdint>
#include <random>
#include <iostream>
#include <fstream>

namespace sampler {

// The simplest reservoir sampling
template <typename T>
struct simple {
  T payload;
  ::std::uint64_t count;
  template <typename G>
  simple(const T& payload, G*) : payload(payload), count(1) {}
  template <typename G>
  void next(const T& x, G* g) {
    auto u = ::std::uniform_int_distribution<uint64_t>(0, count);
    ++count;
    if (0 == u(*g)) payload = x;
  }
};

// Faster sampling using "Reservoir-sampling algorithms of time complexity
// O(n(1+log(N/n)))" by Kim-Hung Li.
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

// template <typename T>
// struct DoubleWordWrapper {
//   using Type = struct NoDoubleWordDefinedFor;
// };

// template <>
// struct DoubleWordWrapper<uint8_t> {
//   using Type = uint16_t;
// };

// template <>
// struct DoubleWordWrapper<uint16_t> {
//   using Type = uint32_t;
// };

// template <>
// struct DoubleWordWrapper<uint32_t> {
//   using Type = uint64_t;
// };

// template <>
// struct DoubleWordWrapper<uint64_t> {
//   using Type = unsigned __int128;
// };

// template <typename T>
// using DoubleWord = typename DoubleWordWrapper<T>::Type;

// template <typename N>
// DoubleWord<N> MultiplyUp(N x, N y) {
//   return DoubleWord<N>(x) * DoubleWord<N>(y);
// }

// template <typename N>
// DoubleWord<N> AsHighBits(N x) {
//   constexpr DoubleWord<N> H = DoubleWord<N>(1) + std::numeric_limits<N>::max();
//   return DoubleWord<N>(x) * H;
// }

// template <typename N>
// DoubleWord<N> AddSmaller(DoubleWord<N> x, N y) {
//   return x + DoubleWord<N>(y);
// }

template <typename N>
struct Ratio {
  N num = 0, den = 1;
};

struct Order {
  enum Ordering { LT, GT, EQ };

  // template <typename N>
  // static Ordering Compare(N x, N y) {
  //   if (x < y) return LT;
  //   if (x > y) return GT;
  //   return EQ;
  // }

  // template <typename N>
  // static Ordering Compare(const std::vector<N>& r, const Ratio<N> p) {
  //   N num = p.num;
  //   //constexpr DoubleWord<N> H = DoubleWord<N>(1) + std::numeric_limits<N>::max();
  //   for (const N v : r) {
  //     const DoubleWord<N> lhs = MultiplyUp(v, p.den), rhs = AsHighBits(num);
  //     switch (Compare(lhs, rhs)) {
  //       case GT: return GT;
  //       case EQ: num = 0; break;
  //       case LT:
  //         if (GT == Compare(AddSmaller(lhs, p.den), rhs)) num = rhs - lhs;
  //         else return LT;
  //     }
  //   }
  //   return num ? LT : EQ;
  // }

  template <typename N>
  static Ordering Compare(const std::vector<bool>& r, const Ratio<N> p) {
    N num = p.num;
    for (const bool v : r) {
      if (v) {
        if (2 * num < p.den) return GT;
        num = 2 * num - p.den;
      } else {
        if (2 * num >= p.den) return LT;
        num = 2 * num;
      }
    }
    return num ? LT : EQ;
  }
};

// template <typename N>
// bool Increment(std::vector<N>* r) {
//   for (uint64_t i = r->size() - 1; i < r->size(); --i) {
//     ++(*r)[i];
//     if ((*r)[i]) return true;
//   }
//   return false;
// }

// template <typename N>
// bool Decrement(std::vector<N>* r) {
//   for (uint64_t i = r->size() - 1; i < r->size(); --i) {
//     --(*r)[i];
//     if (N(~N((*r)[i]))) return true;
//   }
//   return false;
// }

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

// template <typename N>
// long double AsFloating(const std::vector<N>& r) {
//   constexpr long LOG2_BASE =
//       log2(static_cast<long double>(1.0) + std::numeric_limits<N>::max()) + 0.1;
//   long double result = 0.0;
//   for (auto i = r.size() - 1; i < r.size(); --i) {
//     result += std::ldexp(r[i], (i + 1) * -LOG2_BASE);
//   }
//   return result;
// }

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

enum struct Direction { LHS, RHS };

// template <typename CDF, Direction D, typename N>
// N invert(const std::vector<N>& r, N count, N lo, N hi) {
//   const auto cdf = [count](N s) { return CDF::F(count, s); };
//   assert(Order::EQ == Order::Compare(std::vector<N>(), cdf(0)));
//   assert(hi > lo);
//   while (hi - lo > 1) {
//     assert(Order::LT != Order::Compare(r, cdf(lo)));
//     assert(Order::GT != Order::Compare(r, cdf(hi)));
//     const N mid = lo + (hi - lo)/2;
//     switch (Order::Compare(r, cdf(mid))) {
//       case Order::LT: hi = mid; break;
//       case Order::GT: lo = mid; break;
//       case Order::EQ: if (D == Direction::LHS) lo = mid; else return mid;
//     }
//   }
//   assert (hi - lo == 1);
//   assert(Order::LT != Order::Compare(r, cdf(lo)));
//   assert(Order::GT != Order::Compare(r, cdf(hi)));
//   return hi;
// }

template <typename CDF, Direction D, typename N>
N Invert(const std::vector<bool>& r, N count, N lo, N hi) {
  const auto cdf = [count](N s) { return CDF::F(count, s); };
  assert(Order::EQ == Order::Compare(std::vector<bool>(), cdf(0)));
  assert(hi > lo);
  while (hi - lo > 1) {
    assert(Order::LT != Order::Compare(r, cdf(lo)));
    assert(Order::GT != Order::Compare(r, cdf(hi)));
    const N mid = lo + (hi - lo)/2;
    switch (Order::Compare(r, cdf(mid))) {
      case Order::LT: hi = mid; break;
      case Order::GT: lo = mid; break;
      case Order::EQ: if (D == Direction::LHS) lo = mid; else return mid;
    }
  }
  assert (hi - lo == 1);
  assert(Order::LT != Order::Compare(r, cdf(lo)));
  assert(Order::GT != Order::Compare(r, cdf(hi)));
  return hi;
}

std::vector<std::pair<long double, uintmax_t>> samples;

template <typename CDF, typename N, typename R>
N Sample(R* urng, N count) {
  N lo = 0, hi = std::numeric_limits<N>::max()/2;
  thread_local std::bernoulli_distribution rng;
  // std::vector<N> previous;
  //std::cout << "A: ";
  for (std::vector<bool> r(1, rng(*urng));; r.push_back(rng(*urng))) {
    //    if (r.size() == 1) std::cout << (!r.back() ? '#' : '_');
    // std::cout << (r.back() ? '#' : '_');
    lo = Invert<CDF, Direction::LHS>(r, count, lo, hi) - 1;
    assert(lo + 1 != 0);
    if (lo + 1 >= hi) {
      //std::cout << std::endl;
      //std::cout << "B: ";
      for (const bool v : r) {
        //std::cout << (v ? "#" : "_");
      }
      //std::cout << std::endl;
      //std::cout << AsFloating(r) << std::endl;
      samples.push_back(std::make_pair(AsFloating(r), hi));
      return hi - 1;
    }
    // previous = r;
    if (!Increment(r)) {
      const bool d = Decrement(r);
      assert(!d);
      continue;
    }
    hi = Invert<CDF, Direction::RHS>(r, count, lo, hi);
    assert(hi > lo);
    if (hi - lo == 1) {
      //std::cout << std::endl;
      //std::cout << "C: ";
      for (const bool v : r) {
        //std::cout << (v ? "#" : "_");
      }
      //std::cout << std::endl;
      Decrement(r);
      //std::cout << AsFloating(r) << std::endl;
      samples.push_back(std::make_pair(AsFloating(r), hi));
      return hi - 1;
    }
    // previous = r;
    const bool d = Decrement(r);
    assert(d);
  }
}

// TODO: rationals have limited precision based on CDF - difference between consecutive
// values?

template <typename N>
struct VitterCDF {
  static Ratio<N> F(N count, N s) { return {s, N(s + count)}; }
};

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

// template <typename Word, typename G>
// struct LazyRand {
//   explicit LazyRand(G* g) :
//       randgen(g), payload(), uniform() {}
//       G* randgen;
//       std::vector<Word> payload;
//       std::uniform_int_distribution<Word> uniform;
//       Word operator[](size_t n) {
//         if (n < payload.size()) return payload[n];
//         assert(n == payload.size());
//         payload.push_back(uniform(*randgen));
//         return payload.back();
//   }
// };

// template <typename DoubleWord, typename Word, typename G>
// bool Less(LazyRand<Word, G>& r, Word num, Word den) {
//   for (size_t i = 0; true; ++i) {
//     if (0 == num) return false;
//     const DoubleWord rbits = r[i];
//     const DoubleWord rden = rbits * static_cast<DoubleWord>(den);
//     const DoubleWord bignum = static_cast<DoubleWord>(num)
//         << std::numeric_limits<Word>::digits;
//     if (rden < bignum) return true;
//     if (rden > bignum) return false;
//     num = bignum % static_cast<DoubleWord>(den);
//   }
// }

// template <typename DoubleWord, typename Word, typename G>
// Word FindSkip(LazyRand<Word, G>& r, Word count) {
//   const DoubleWord bigr = r[0], bigcount = count;
//   constexpr DoubleWord MAX_WORD = static_cast<Word>(~static_cast<Word>(0));
//   const DoubleWord negr = MAX_WORD + 1 - bigr;
//   DoubleWord lo = (bigr * bigcount) / negr;
//   // if (lo > 0) --lo;
//   DoubleWord hi = MAX_WORD - bigcount;
//   if (negr - 1 > 0) {
//     //hi = std::min(hi, static_cast<DoubleWord>((bigcount * (bigr + 1)) / (negr - 1)));
//   }
//   //if (lo > hi) return hi - 1;
//   Word down = lo, up = hi;
//   down = 0;
//   while (up - down > 1) {
//     // std::cout << std::hex << "(" << static_cast<std::uintmax_t>(down) << ", "
//     //           << static_cast<std::uintmax_t>(up) << ") = " << std::dec
//     //           << static_cast<std::uintmax_t>(up - down) << std::endl;
//     const Word mid = down + (up - down) / 2;
//     //    if (Less<DoubleWord, Word>(r, mid +1, mid + count + 1)) up = mid;
//     if (Less<DoubleWord, Word>(r, mid, mid + count)) up = mid;
//     else down = mid;
//   }
//   return down;
// }

template <typename T>
struct Vitter {
  // using Word = uint64_t; using DoubleWord = unsigned __int128;
  //using Word = uint8_t; //using DoubleWord = uint16_t;
  using Word = uint64_t;
  T payload;
  Word count, skip;
  template <typename G>
  explicit Vitter(const T& payload, G* g)
    : payload(payload), count(1), skip(Sample<VitterCDF<Word>>(g, count)) {}
  template <typename G>
  void next(const T& x, G* g) {
    ++count;
    if (skip) {
      --skip;
    } else {
      payload = x;
      skip = Sample<VitterCDF<Word>>(g, count);
    }
  }
};
}
