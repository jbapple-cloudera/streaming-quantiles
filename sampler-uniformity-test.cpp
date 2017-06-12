#include "sampler.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <future>

using namespace std;
using namespace sampler;

template <template<typename N> typename Sampler>
void Uniformity(size_t width, size_t count) {
  // DevUrandom<uint64_t> randgen;
  random_device randgen;
  vector<size_t> result(width, 0);
  for (size_t i = 0; i < count; ++i) {
    Sampler<unsigned __int128> sampler;
    size_t payload = width;
    for (size_t j = 0; j < width; ++j) {
      if (sampler.Step(&randgen)) payload = j;
    }
    ++result.at(payload);
    if (i % 1'000'000 == 0) {
      cout << decltype(sampler)::NAME() << ' ' << right << setw(4) << i / 1'000'000
           << ' ';
      long double max_quality = 0.0;
      for (auto r : result) {
        max_quality = std::max(max_quality, -log2(std::abs((r / (long double)i) - (1.0 / width))));
      }
      cout << left << setw(8) << max_quality << " " << endl;
    }
  }
}


int main() {
  constexpr size_t width = 96, count = numeric_limits<size_t>::max();
  auto f1 = async(launch::async, Uniformity<Li>, width, count);
  auto f2 = async(launch::async, Uniformity<Simple>, width, count);
  auto f3 = async(launch::async, Uniformity<Vitter>, width, count);
}
