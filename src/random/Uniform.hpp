/* @file Uniform.hpp */

#pragma once

#include "Generator.hpp"
#include <random>

namespace xo {
  namespace random {
    // Generate pseudorandom numbers uniformly on [0.0, 1.0]
    //
    template<class Engine>
    class UnitIntervalGen {
    public:
      using engine_type = Engine;
      using generator_type = Generator<Engine,
				       std::uniform_real_distribution<double>>;

      template<typename Seed>
      static generator_type make(Seed const & seed) {
	return generator_type::make(Engine(seed),
				    std::uniform_real_distribution<double>(0.0, 1.0));
      }
    }; /*UnitIntervalGen*/

  } /*namespace random*/
} /*namespace xo*/

/* end Uniform.hpp */
