#include "sampled-kll.hpp"
#include "utility.hpp"
#include "kll.hpp"

int main(int argc, char** argv) {
  SampledKll<string, 5> sketch;
  assert(argc == 2);
  //Benchmark<UrandomBool, SampledKll<string, 200>>(argv[1]);
  //Benchmark<UrandomBool, Kll<string, 1000>>(argv[1]);
  InteractiveTest<UrandomBool, SampledKll<string, 200>, SampledKll<string, 1000>>(
      argv[1]);
}
