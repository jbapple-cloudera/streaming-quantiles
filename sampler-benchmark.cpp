#include "sampler.hpp"
#include "utility.hpp"

#include <iostream>
using namespace std;
using namespace sampler;

int main(int argc, char ** argv) {
  assert (3 == argc);
  const auto many = StringCast<uint64_t>(argv[1]);
  const auto limit = StringCast<uint64_t>(argv[2]);
  ::std::random_device g1;
  ::std::mt19937_64 g2;
  DevUrandom<uint64_t> g3;
  uint64_t total = 0;
  for (uint64_t i = 0; i < many; ++i) {
    //sampler::Simple<unsigned __int128> s2;
    //sampler::Li<uint64_t> s2;
    sampler::Vitter<uint64_t> s2;
    for (uint64_t j = 1; j < limit; ++j) {
      total += s2.Step(&g1);
      //s1.next(j, &g1);
    }
  }
  cout << total << endl;
}
