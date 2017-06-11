#include "sampler.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>

using namespace std;
using namespace sampler;

template <template<typename> typename Sampler>
vector<size_t> Uniformity(size_t width, size_t count) {
  //std::random_device randgen;
  //sampler::PromptPrng<uint8_t> randgen;
  DevUrandom<uint64_t> randgen[3];
  //sampler::DevUrandomBuffer<uint64_t> randgen(1 << 10);

  vector<size_t> result[3] = {
      vector<size_t>(width, 0), vector<size_t>(width, 0), vector<size_t>(width, 0)};
  for (size_t i = 0; i < count; ++i) {
    //Sampler<size_t> sampler(0, &randgen);
    sampler::exp<size_t, float> sampler0(0, &randgen[0]);
    sampler::exp<size_t, long double> sampler1(0, &randgen[1]);
    Vitter<size_t> sampler2(0, &randgen[2]);
    for (size_t j = 1; j < width; ++j) {
      sampler0.next(j, &randgen[0]);
      sampler1.next(j, &randgen[1]);
      sampler2.next(j, &randgen[2]);
    }
    ++result[0][sampler0.payload];
    ++result[1][sampler1.payload];
    ++result[2][sampler2.payload];
    if (i % 400'000 == 0) {
      //cout << i << ' ';
      for (int j = 0; j < 3; ++j) {
        for (auto r : result[j]) {
          cout << left << setw(8) << -log2(std::abs((r / (long double)i) - (1.0 / width)))
               << " ";
        }
        for (auto r : result[j]) {
          cout << left << setw(8) << 100 * r / (long double)i << " ";
        }
        cout << endl;
      }
      cout << endl;
    }
  }
  // for (auto r : result) {
  //   cout << 100 * r / (long double)count << " ";
  // }
  // cout << endl;
  return result[0];
}

// template<typename... Samplers>//, typename UrngTuple>
// void UniformPack(size_t width, size_t count){//, UrngTuple* urngs) {
//   int ok[sizeof...(Samplers)] = {(Uniformity<Samplers>(width, count),0)...};
// }

int main() {
  constexpr size_t width = 2, count = numeric_limits<size_t>::max();
  using Samplers = tuple<simple<size_t>, Vitter<size_t>>;
  // auto urngs = forward_as_tuple(
  //     random_device(), DevUrandom<uint64_t>(), DevUrandomBuffer<uint64_t>(1 << 10));
  //UniformPack<simple<size_t>, Vitter<size_t>>(width, count);
  Uniformity<simple>(width, count);
  //  Uniformity<Vitter>(width, count);
  //Uniformity<sampler::exp>(width, count);

  // if (0) {
  //   auto result = Uniformity<sampler::simple<size_t>>(width, count);
  //   for (auto r : result) {
  //     cout << r << " ";
  //   }
  //   cout << endl;
  //   return 0;
  //   }
  //   if (1) {
  //     auto result = Uniformity<sampler::Vitter<size_t>>(width, count);
  //     for (auto r : result) {
  //       cout << r << " ";
  //     }
  //     cout << endl;
  //     return 0;
  //   }
  //   auto result = Uniformity<sampler::exp<size_t>>(width, count);
  //   for (auto r : result) {
  //     cout << r << " ";
  //   }
  //   cout << endl;
  //   return 0;
  
}
