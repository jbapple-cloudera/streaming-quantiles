#include "sampler.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

template<typename Sampler>
std::vector<size_t> Uniformity(size_t width, size_t count) {
  std::random_device randgen;
  //sampler::PromptPrng<uint8_t> randgen;
  //sampler::DevUrandom<uint64_t> randgen;
  //sampler::DevUrandomBuffer<uint64_t> randgen(1 << 10);
  std::vector<size_t> result(width, 0);
  for (int i = 0; i < count; ++i) {
    //sampler::ExplodingPrng<uint8_t> randgen(i);
    try {
      Sampler sampler(0, &randgen);
      for (size_t j = 1; j < width; ++j) {
        sampler.next(j, &randgen);
        // cout << "< " << (uintmax_t)sampler.skip << endl;
      }
      // cout << "<< " << sampler.payload << endl;
      ++result[sampler.payload];
    } catch (...) {}
  }
  //sort(sampler::samples.begin(), sampler::samples.end());
  // for (auto s : sampler::samples) cout << "(" << s.first << ", " << s.second << ") ";
  sampler::samples.clear();
  cout << endl;
  return result;
}

int main() {
  while (true) {
    uint64_t width, count;
    //cout << "> ";
    //if (!(cin >> width)) break;
    //if (!(cin >> count)) break;
    width = 4;
    count = 44'444'444;
    if (0) {
      auto result = Uniformity<sampler::simple<size_t>>(width, count);
      for (auto r : result) {
        cout << r << " ";
      }
      cout << endl;
      return 0;
    }
    if (1) {
      auto result = Uniformity<sampler::Vitter<size_t>>(width, count);
      for (auto r : result) {
        cout << r << " ";
      }
      cout << endl;
      return 0;
    }
    auto result = Uniformity<sampler::exp<size_t>>(width, count);
    for (auto r : result) {
      cout << r << " ";
    }
    cout << endl;
    return 0;
  }
}
