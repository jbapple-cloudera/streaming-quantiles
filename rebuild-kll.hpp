#include "utility.hpp"
#include "sampler.hpp"

template<typename T, typename Sampler = sampler::exp<T>>
struct RebuildKll {
  Sampler sampler;
  int64_t sample_weight;
  int8_t sample_height;
  unique_ptr<T[]> payload;
  array<?> sizes;
  array<?> start_locations;
};
