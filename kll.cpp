
int main(int argc, char ** argv) {
  SampledKll<string, 28> x;
  assert (argc == 2);
  Benchmark<UrandomBool, SampledKll<string, 28>>(argv[1]);

  PrintKllArray<19>();
  PrintKllArray<20>();
  PrintKllArray<28>();

  // PrintKllArray<200>();
  // PrintKllArray<400>();
  // PrintKllArray<800>();
  // PrintKllArray<1600>();
  // PrintKllArray<3200>();

  // PrintKllArray<201>();
  // PrintKllArray<202>();
  // PrintKllArray<203>();
  // PrintKllArray<204>();
  // PrintKllArray<205>();
  // PrintKllArray<206>();
  // PrintKllArray<207>();
  // PrintKllArray<208>();
  // PrintKllArray<209>();
  // PrintKllArray<210>();
  // PrintKllArray<211>();
  // PrintKllArray<212>();
  // PrintKllArray<213>();
  // PrintKllArray<214>();
  // PrintKllArray<215>();
  // PrintKllArray<216>();
  // PrintKllArray<217>();

  // Quality<UrandomBool, Kll<string, 1000>, Reservoir<string, 20000>>(argv[1]);
  // InteractiveTest<UrandomBool, Reservoir<string, 1000>>(argv[1]);
  // PrintTimer([&] { Benchmark<UrandomBool, Reservoir<string, 20000>>(argv[1]); return 0; });
  // PrintTimer([&] { Benchmark<UrandomBool, Kll<string, 1000>>(argv[1]); return 0; });
  // PrintTimer([&] { Benchmark<UrandomBool, Reservoir<string, 20000>>(argv[1]); return 0; });
}
