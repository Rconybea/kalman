/* @file PricingContext.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "time/Time.hpp"

namespace xo {
  namespace option {
    /* option pricing context --
     * input ingredients for pricing any of a set of related options
     * on the same underlying
     */
    class PricingContext : public ref::Refcount
    {
    public:
      using utc_nanos = time::utc_nanos;
      
      static ref::rp<PricingContext> make(utc_nanos t0,
					  double spot,
					  double volatility,
					  double rate);

      utc_nanos t0() const { return t0_; }
      double spot() const { return spot_; }
      double volatility() const { return volatility_; }
      double rate() const { return rate_; }

    private:
      PricingContext(utc_nanos t0, double spot, double volatility, double rate)
	: t0_{t0}, spot_{spot}, volatility_{volatility}, rate_{rate} {}

    private:
      /* current time */
      utc_nanos t0_;
      /* underlying spot price */
      double spot_ = 0.0;
      /* volatility (proxy for volatiliy surface) */
      double volatility_ = 0.0;
      /* interest rate (proxy for yield curve) */
      double rate_ = 0.0;
    }; /*PricingContext*/
  } /*namespace option*/
} /*namespace xo*/

/* end PricingContext.hpp */
