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
  //std::random_device randgen;
  //sampler::PromptPrng<uint8_t> randgen;
  DevUrandom<uint64_t> randgen;
  //sampler::DevUrandomBuffer<uint64_t> randgen(1 << 10);

  vector<size_t> result(width, 0);

  for (size_t i = 0; i < count; ++i) {
    Sampler<unsigned __int128> sampler;
    size_t payload = width;
    for (size_t j = 0; j < width; ++j) {
      if (sampler.Step(&randgen)) payload = j;
    }
    ++result.at(payload);
    if (i % 2'000'000 == 0) {
      //cout << i << ' ';
      cout << decltype(sampler)::NAME() << ' ';
      for (auto r : result) {
        cout << left << setw(8) << -log2(std::abs((r / (long double)i) - (1.0 / width)))
             << " ";
      }
      for (auto r : result) {
        cout << left << setw(8) << 100 * r / (long double)i << " ";
      }
      cout << endl;
    }
  }
}


int main() {
  constexpr size_t width = 2, count = numeric_limits<size_t>::max();
  //auto f1 = async(launch::async, Uniformity<Li>, width, count);
  auto f2 = async(launch::async, Uniformity<Simple>, width, count);
  auto f3 = async(launch::async, Uniformity<Vitter>, width, count);
}
