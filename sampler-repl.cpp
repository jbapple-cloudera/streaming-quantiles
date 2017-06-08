#include "sampler.hpp"

#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

int main() {
  while (true) {
    uint64_t count;
    cout << "> ";
    if (!(cin >> count)) break;
    long double sum = 0, log_sum = 0, inv_sum = 0;
    int i = 0;
    std::random_device randgen;
    for (; i < (1 << 5); ++i) {
      // using DoubleWord = unsigned __int128;
      // using Word = uint64_t;
      using DoubleWord = uint16_t;
      using Word = uint8_t;
      sampler::LazyRand<Word, std::random_device> r(&randgen);
      // sampler::LazyRand<Word, sampler::PromptPrng<Word>> r;
      const auto skip = sampler::FindSkip<DoubleWord, Word>(r, count);
      sum += skip;
      log_sum += log(1 + static_cast<long double>(skip));
      inv_sum += 1.0 / (1 + static_cast<long double>(skip));
      if (false && 0 == (i & (i + 1))) {
        cout << setprecision(2) << fixed << (sum / (i + 1)) << ", "
             << exp(log_sum / (i + 1)) - 1 << ", " << 1.0 / (inv_sum / (i + 1)) - 1
             << endl;
      }
    }
    cout << setprecision(2) << fixed << (sum / i) << ", " << exp(log_sum / i) - 1 << ", "
         << 1.0 / (inv_sum / i) - 1 << endl;
  }
}
