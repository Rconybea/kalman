/* @file Normal.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    /* Engine:
     *  std::mt19937
     *  xo::random::xoshiro256
     */
    template<class Engine>
    class NormalGen {
    public:
      using engine_type = Engine;
      using generator_type
	= Generator<Engine,
		    std::normal_distribution<double>>;

      template<typename Seed>
      static generator_type make(Seed const & seed, double mean, double sdev) {
	return generator_type::make(Engine(seed),
				    std::normal_distribution(mean, sdev));
      }
    }; /*NormalGen*/
  } /*namespace random*/
} /*namespace xo*/

/* end Normal.hpp */
