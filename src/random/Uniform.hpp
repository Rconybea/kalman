/* @file Uniform.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    // Generate pseudorandom numbers uniformly on [0.0, 1.0]
    //
    class UnitIntervalGen {
    public:
      using generator_type = Generator<std::mt19937,
				       std::uniform_real_distribution<double>>;

      static generator_type make(uint64_t seed) {
	return generator_type::make(std::mt19937(seed),
				    std::uniform_real_distribution<double>(0.0, 1.0));
      }
    }; /*UnitIntervalGen*/

  } /*namespace random*/
} /*namespace xo*/

/* end Uniform.hpp */
