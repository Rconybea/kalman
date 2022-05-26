/* @file Generator.hpp */

#pragma once

#include <utility>

namespace xo {
  namespace random {
    template <class Engine, class Distribution>
    class Generator {
    public:
      using result_type = typename Distribution::result_type;

    public:
      Generator(Engine & e, Distribution const & d)
	: engine_{e},
	  distribution_{d} {}
      Generator(Engine && e, Distribution && d)
	: engine_{std::move(e)},
	  distribution_{std::move(d)} {}

      static Generator make(Engine && e, Distribution && d) {
	return Generator(e, d);
      }

      result_type operator()() { return this->distribution_(this->engine_); }

    private:
      /* random number generator;  generates uniformly-distributed integers */
      Engine engine_;
      /* distribution object */
      Distribution distribution_;
    }; /*Generator*/

  } /*namespace random*/
} /*namespace xo*/

/* end Generator.hpp */
