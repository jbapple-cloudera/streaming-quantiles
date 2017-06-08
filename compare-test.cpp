#include "sampler.hpp"

#include <cassert>
#include <iostream>

using namespace std;

int main() {
  for (int i = 0; i <64; ++i) {
    vector<uint8_t> r(i, 0);
    do {
      const auto r_float = sampler::AsFloating(r);
      for (uint8_t den = 1; den > 0; ++den) {
        for (uint16_t num = 0; num <= den; ++num) {
          const sampler::Ratio<uint8_t> f = {static_cast<uint8_t>(num), den};
          const auto expected = sampler::Order::Compare(r_float, sampler::AsFloating(f));
          const auto actual = sampler::Order::Compare(r, f);
          assert(expected == actual);
        }
      }
    } while(sampler::Increment(&r));
    cout << "OK " << i << endl;
  }
}
