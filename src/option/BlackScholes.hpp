/* @file BlackScholes.hpp */

#include "option/VanillaOption.hpp"
#include "option/PricingContext.hpp"
#include "option/Greeks.hpp"
#include "option_util/Callput.hpp"
#include "distribution/Normal.hpp"
#include <cmath>

namespace xo {
  namespace option {
    /* black-scholes option pricing model */
    class BlackScholes {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* with:
       *   N(d)  = cumulative normal distribution function
       *   N'(d) = normal density function
       *
       *   S     = spot price of underlying
       *   D     = exp(-r.t) = discount factor to option expiry
       *   F     = S/D = forward price at option expiry
       *
       *   K     = option strike price
       *   r     = risk-free interest rate
       *   t     = time to expiry
       *   s     = volatility of underlying
       *
       *           ln(S/K) + (r + s^2/2).t
       *   d1    = -----------------------
       *                  s.sqrt(t)
       *
       *   d2    = d1 - s.sqrt(t)
       *
       * value:
       * - call-option:
       *     N(d1).S - N(d2).K.exp(-r.t)
       * - put-option:
       *     N(-d2).K.exp(-r.t) - N(-d1).S
       *
       * delta:
       * - call-option:
       *     N(d1)
       * - put-option:
       *     -N(-d1) = N(d1) - 1  -> N(-d1) = 1 - N(d1)
       *
       * gamma (2nd derivative w.r.t. S):
       *     N'(d1) / (S.s.sqrt(t))
       *
       * vega (derivative w.r.t. s):
       *     S.N'(d1).sqrt(t)
       *
       * theta (-1 * derivative w.r.t. t):
       * - call-option:
       *     - (S.N'(d1).s / 2.sqrt(t)) - r.K.exp(-r.t).N(d2)
       * - put-option:
       *     - (S.N'(d1).s / 2.sqrt(t)) + r.K.exp(-r.t).N(-d2)
       *
       * rho (derivative w.r.t. r)
       * - call-option:
       *     +K.t.exp(-r.t).N(d2)
       * - put-option:
       *     -K.t.exp(-r.t).N(-d2)
       */
      static Greeks greeks(Callput pc, double K, double S, double s, double r, double t);

      /* variation,  using VanillaOption for (callput, strike, expiry) */
      static Greeks greeks(ref::brw<VanillaOption> opt,
			   double S, double s, double r, utc_nanos t0);

      /* variation,  using:
       *  VanillaOption for (callput, strike, expiry)
       *  PricingContext for (s, r)
       */
      static Greeks greeks(ref::brw<VanillaOption> opt,
			   ref::brw<PricingContext> pcx,
			   double ul_spot,
			   utc_nanos t0);
    }; /*BlackScholes*/
  } /*namespace option*/
} /*namespace xo*/

/* end BlackScholes.hpp */
