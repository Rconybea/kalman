/* @file Exponential.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    class ExponentialGen {
    public:
      using generator_type
	= Generator<std::mt19937,
		    std::exponential_distribution<double>>;

      static generator_type make(uint64_t seed, double lambda) {
	return generator_type::make(std::mt19937(seed),
				    std::exponential_distribution<double>(lambda));
      } /*make*/
    }; /*ExponentialGen*/
  } /*namespace random*/
} /*namespace xo*/

/* end Exponential.hpp */
