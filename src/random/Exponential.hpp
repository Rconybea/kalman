/* @file Exponential.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    /* generate exponentially-distributed random numbers
     * with some half-life lambda
     */
    template<class Engine>
    class ExponentialGen {
    public:
      using engine_type = Engine;
      using generator_type
	= Generator<Engine,
		    std::exponential_distribution<double>>;

      template<class Seed>
      static generator_type make(Seed const & seed, double lambda) {
	return generator_type::make(Engine(seed),
				    std::exponential_distribution<double>(lambda));
      } /*make*/
    }; /*ExponentialGen*/
  } /*namespace random*/
} /*namespace xo*/

/* end Exponential.hpp */
