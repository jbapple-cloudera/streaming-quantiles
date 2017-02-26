#include "sampled-kll.hpp"
#include "utility.hpp"
#include "kll.hpp"

using namespace std;

int main(int argc, char** argv) {
  SampledKll<string, 5> sketch;
  assert(argc == 2);
  //Benchmark<UrandomBool, SampledKll<string, 200>>(argv[1]);
  //Benchmark<UrandomBool, Kll<string, 1000>>(argv[1]);
  InteractiveTest<random_device, Kll<string, 1024>, SampledKll<string, 1024>>(
      argv[1]);
}
