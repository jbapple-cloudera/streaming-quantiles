#include "sampler.hpp"

#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

template<typename Sampler>
std::vector<size_t> Uniformity(size_t width, size_t count) {
  //std::random_device randgen;
  //sampler::PromptPrng<uint8_t> randgen;
  std::vector<size_t> result(width, 0);
  for (int i = 0; i < count; ++i) {
    sampler::ExplodingPrng<uint8_t> randgen(i);
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
  return result;
}

int main() {
  while (true) {
    uint64_t width, count;
    cout << "> ";
    if (!(cin >> width)) break;
    if (!(cin >> count)) break;
    auto result = Uniformity<sampler::Vitter<size_t>>(width, count);
    for (auto r : result) {
      cout << r << " ";
    }
    cout << endl;
    //result = Uniformity<sampler::exp<size_t>>(width, count);
    //for (auto r : result) {
    //  cout << r << " ";
    //}
    cout << endl;
  }
}
