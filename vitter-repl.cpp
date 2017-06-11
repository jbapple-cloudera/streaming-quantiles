#include "sampler.hpp"

#include <iostream>

using namespace std;

int main() {
  random_device d;
  uintmax_t in;
  cout << "> ";
  while (cin >> in) {
    cout << uintmax_t(sampler::Sample<sampler::VitterCDF<uint8_t>, uint8_t>(&d, in))
         << endl
         << "> ";
  }
}
