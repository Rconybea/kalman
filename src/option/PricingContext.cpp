/* @file PricingContext.cpp */

#include "PricingContext.hpp"

namespace xo {
  namespace option {
    ref::rp<PricingContext>
    PricingContext::make(utc_nanos t0,
			 double spot,
			 double volatility,
			 double rate)
    {
      return new PricingContext(t0, spot, volatility, rate);
    } /*make*/
  } /*namespace option*/
} /*namespace xo*/

/* end PricingContext.cpp */
