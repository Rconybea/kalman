/* @file Bernoulli.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    class BernoulliGen {
    public:
      using generator_type
	= Generator<std::mt19937,
		    std::bernoulli_distribution>;

      static generator_type make(uint64_t seed, double prob) {
	return generator_type::make(std::mt19937(seed),
				    std::bernoulli_distribution(prob));
      }

      static generator_type coinflip(uint64_t seed) {
	return make(seed, 0.5 /*prob*/);
      }
    }; /*BernoulliGen*/
  } /*namespace random*/
} /*namespace xo*/

/* end Bernoulli.hpp */
