/* @file Price.hpp */

#pragma once

#include <cstdint>

namespace xo {
  namespace option {
    /* price with exact representation */
    class Price {
    public:
      Price() = default;

      static Price from_double(double px) { return Price(px * sc_inv_unit); }

      double to_double() const { return sc_unit * rep_; }

    private:
      explicit Price(int64_t rep) : rep_{rep} {}

      static constexpr double sc_unit = 0.0001;
      static constexpr double sc_inv_unit = (1.0 / sc_unit);

    private:
      /* r = .rep represents a price
       *    c_unit * r
       *
       * e.g.
       *   rep=10000 -> price = $1
       *   rep= 1000 -> price = $0.10
       *   rep=  100 -> price = $0.01
       */
      int64_t rep_ = 0;
    }; /*Price*/
  } /*namespace option*/
} /*namespace xo*/

/* end Price.hpp */


   
