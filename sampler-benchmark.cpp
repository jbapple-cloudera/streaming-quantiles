#include "sampler.hpp"
#include "utility.hpp"

int main(int argc, char ** argv) {
  assert (3 == argc);
  const auto many = StringCast<uint64_t>(argv[1]);
  const auto limit = StringCast<uint64_t>(argv[2]);
  ::std::random_device g1;
  ::std::mt19937_64 g2;
  for (uint64_t i = 0; i < many; ++i) {
    //sampler::simple<uint64_t> s1(0);
    sampler::exp<uint64_t> s2(0, &g1);
    for (uint64_t j = 1; j < limit; ++j) {
      s2.next(j, &g1);
      //s1.next(j, &g1);
    }
  }
}
