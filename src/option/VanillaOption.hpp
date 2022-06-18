/* @file VanillaOption.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "option/OptionId.hpp"
#include "option/Callput.hpp"
#include "option/Pxtick.hpp"
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
      static ref::rp<VanillaOption> make(OptionId id, Callput cp, double k, utc_nanos x, Pxtick pxtick) {
	return new VanillaOption(id, cp, k, x, pxtick);
      } /*make*/
	
      Callput callput() const { return callput_; }
      double stated_strike() const { return strike_; }
      utc_nanos expiry() const { return expiry_; }
      Pxtick pxtick() const { return pxtick_; }

      /* ignoring pxmult/delivmult detail for now */
      uint32_t pxmult() const { return 100; }
      uint32_t delivmult() const { return 100; }
      double effective_strike() const { return strike_; }

      /* convert a per-share quantity to per-contract quantity */
      double sh2ct(double x) const { return x * this->delivmult(); }
      /* convert a screen quantity to per-contract quantity */
      double px2ct(double x) const { return x * this->pxmult(); }
      /* convert a per-share quantity to screen units */
      double sh2px(double x) const { return x * this->delivmult() / this->pxmult(); }

    private:
      VanillaOption() = default;
      VanillaOption(OptionId id, Callput cp, double k, utc_nanos x, Pxtick pxtick)
	: id_{id}, callput_{cp}, strike_{k}, expiry_{x}, pxtick_(pxtick) {}

    private:
      /* unique id# for this option */
      OptionId id_;
      /* call|put */
      Callput callput_ = Callput::call;
      /* option strike price */
      double strike_ = 0.0;
      /* expiration time */
      utc_nanos expiry_;
      /* tick size encoding */
      Pxtick pxtick_ = Pxtick::all_penny;
    }; /*VanillaOption*/
  } /*namespace option*/
} /*namespace xo*/

/* end VanillaOption.hpp */
