/* @file BlackScholes.cpp */

#include "BlackScholes.hpp"
#include "logutil/scope.hpp"

namespace xo {
  namespace option {
    Greeks
    BlackScholes::call_greeks(double K, double S, double s, double r, double t)
    {
      using xo::distribution::Normal;
      using logutil::scope;
      using logutil::xtag;
      
      constexpr char const * c_self = "BlackScholes::call_greeks";
      constexpr bool c_logging_enabled = true;

      scope lscope(c_self, c_logging_enabled);

      /* s = volatility.   units are 1/sqrt(t)
       * t = time to expiry
       */

      /* s_root_t: s.sqrt(t) */
      double s_root_t = s * ::sqrt(t);

      /* s^2/2 */
      double half_s2 = 0.5 * s * s;

      /* S/K */
      double log_sk = ::log(S/K);

      /*
       *           ln(S/K) + (r + s^2/2).t
       *   d1    = -----------------------
       *                  s.sqrt(t)
       */
      double d1 = (log_sk + (r + half_s2) * t) / s_root_t;

      /*   d2    = d1 - s.sqrt(t) */
      double d2 = d1 - s_root_t;

      /* discount factor to option expiry */
      double D = ::exp(-r * t);

      /* forward price of underlying */
      double F = S / D;

      /* convenience lambda */
      auto N = [](double x) { return Normal::cdf_impl(x); };

      /* option value */
      double tv = (N(d1) * S) - (N(d2) * K * D);

      if(c_logging_enabled) {
	lscope.log(xtag("K", K),
		   xtag("S", S),
		   xtag("s", s),
		   xtag("r", r),
		   xtag("t", t),
		   xtag("s.sqrt(t)", s_root_t),
		   xtag("(s^2)/2", 0.5 * s * s),
		   xtag("ln(S/K)", log_sk),
		   xtag("d1", d1),
		   xtag("d2", d2),
		   xtag("D", D),
		   xtag("F", F),
		   xtag("N(d1)", N(d1)),
		   xtag("N(d2)", N(d2)),
		   xtag("tv", tv)
		   );
      }

      return Greeks(tv);
    } /*call_greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end BlackScholes.cpp */
