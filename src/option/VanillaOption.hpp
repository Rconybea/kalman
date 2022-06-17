/* @file VanillaOption.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "option/Callput.hpp"
#include "time/Time.hpp"

namespace xo {
  namespace option {
    /* minimal representation for option terms
     * just putting enough information here to drive model pricing
     */
    class VanillaOption : public ref::Refcount {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      static ref::rp<VanillaOption> make(Callput cp, double k, utc_nanos x) {
	return new VanillaOption(cp, k, x);
      } /*make*/
	
      Callput callput() const { return callput_; }
      double stated_strike() const { return strike_; }
      utc_nanos expiry() const { return expiry_; }

      /* ignoring pxmult/delivmult detail for now */
      uint32_t pxmult() const { return 100; }
      uint32_t delivmult() const { return 100; }
      double effective_strike() const { return strike_; }

    private:
      VanillaOption() = default;
      VanillaOption(Callput cp, double k, utc_nanos x)
	: callput_{cp}, strike_{k}, expiry_{x} {}

    private:
      /* call|put */
      Callput callput_ = Callput::call;
      /* option strike price */
      double strike_ = 0.0;
      /* expiration time */
      utc_nanos expiry_;
    }; /*VanillaOption*/
  } /*namespace option*/
} /*namespace xo*/

/* end VanillaOption.hpp */
