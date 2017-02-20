#include "utility.hpp"
#include "kll.hpp"
#include "reservoir.hpp"
#include "sampled-kll.hpp"

int main(int argc, char ** argv) {
  assert (argc == 2);
  //InteractiveTest<UrandomBool, SampledKll<string, 1000>>(argv[1]);
  //InteractiveTest<UrandomBool, SampledKll<string, 1000>>(argv[1]);
  Quality<UrandomBool, SampledKll<string, 1000>, Kll<string, 1000>,
      Reservoir<string, 1000>, Reservoir<string, 20000>>(argv[1]);
  // InteractiveTest<UrandomBool, Reservoir<string, 1000>>(argv[1]);
  //PrintTimer([&] { Benchmark<UrandomBool, Reservoir<string, 20000>>(argv[1]); return 0; });
  //PrintTimer([&] { Benchmark<UrandomBool, Kll<string, 1000>>(argv[1]); return 0; });
  //PrintTimer([&] { Benchmark<UrandomBool, SampledKll<string, 1000>>(argv[1]); return 0; });

}
