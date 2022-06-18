/* @file Price.hpp */

#pragma once

#include <cstdint>

namespace xo {
  namespace option {
    /* price with exact representation
     * note that prices can be negative (for complex products)
     */
    class Price {
    public:
      Price() = default;

      static constexpr Price from_double(double px) { return Price(px * sc_inv_unit); }
      static Price from_rep(int32_t rep) { return Price(rep); }

      static int32_t compare(Price x, Price y) { return x.rep() - y.rep(); }

      int32_t rep() const { return rep_; }
      double to_double() const { return sc_unit * rep_; }

    private:
      explicit constexpr Price(int32_t rep) : rep_{rep} {}

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
      int32_t rep_ = 0;
    }; /*Price*/

    inline bool operator==(Price x, Price y) { return Price::compare(x, y) == 0; }
    inline bool operator!=(Price x, Price y) { return Price::compare(x, y) != 0; }
    inline bool operator< (Price x, Price y) { return Price::compare(x, y) <  0; }
    inline bool operator<=(Price x, Price y) { return Price::compare(x, y) <= 0; }
    inline bool operator> (Price x, Price y) { return Price::compare(x, y) >  0; }
    inline bool operator>=(Price x, Price y) { return Price::compare(x, y) >= 0; }
    
    inline Price operator-(Price x, Price y) { return Price::from_rep(x.rep() - y.rep()); }
  } /*namespace option*/
} /*namespace xo*/

/* end Price.hpp */


   
