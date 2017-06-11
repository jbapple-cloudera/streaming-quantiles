#include "sampler.hpp"

#include <cassert>
#include <iostream>

using namespace std;

int main() {
  for (int i = 0; i < 20; ++i) {
    vector<bool> r(i, false);
    do {
      const auto r_float = sampler::AsFloating(r);
      for (uint8_t den = 1; den > 0; ++den) {
        for (uint16_t num = 0; num <= den; ++num) {
          const sampler::Ratio<uint8_t> f = {static_cast<uint8_t>(num), den};
          const auto f_float = sampler::AsFloating(f);
          const auto expected = (r_float < f_float) ?
              sampler::Order::LT :
              (r_float > f_float) ? sampler::Order::GT : sampler::Order::EQ;
          const auto actual = sampler::Order::Compare(r, f);
          if (expected != actual) {
            cerr << r_float << ' ' << f_float << ' ' << (int)den << ' ' << num << endl;
            return 1;
          }
        }
      }
    } while(sampler::Increment(r));
    cout << "OK " << i << endl;
  }
}
