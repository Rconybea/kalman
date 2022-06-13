/* @file BlackScholes.cpp */

#include "BlackScholes.hpp"
#include "logutil/scope.hpp"

namespace xo {
  namespace option {
    Greeks
    BlackScholes::greeks(Callput pc, double K, double S, double s, double r, double t)
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

      /* root_t: sqrt(t) */
      double root_t = ::sqrt(t);

      /* s_root_t: s.sqrt(t) */
      double s_root_t = s * root_t;

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

      /* N(d1) */
      double Nd1 = N(d1);
      /* N(-d1) (= 1-N(d1) by symmetry of normal density function around y-axis) */
      double Nmd1 = 1.0 - Nd1;
      /* N'(d1) */
      double nd1 = Normal::density(d1);
      /* N(d2) */
      double Nd2 = N(d2);
      /* N(-d2) */
      double Nmd2 = 1.0 - Nd2;

      /* tv: option value */
      double tv = 0.0;
      double delta = 0.0;

      switch(pc) {
      case Callput::call:
	tv = (Nd1 * S) - (Nd2 * K * D);
	delta = Nd1;
	break;
      case Callput::put:
	tv = -(Nmd1 * S) + (Nmd2 * K * D);
	delta = -Nmd1;
	break;
      }

      double gamma = nd1 / (S * s_root_t);
      /* vega: S.N'(d1).sqrt(t) */
      double vega = S * nd1 * root_t;
      /* theta: (S.N'(d1).s / 2.sqrt(t)) - r.K.exp(-r.t).N(d2)
       *   (reverse sign,  bc want theta as ttx gets smaller)
       */
      double theta = (-0.5 * S * nd1 * s / root_t);
      double rho = 0.0;

      switch(pc) {
      case Callput::call:
	theta += (r * K * D * Nd2);
	/* rho: K.t.exp(-r.t).N(d2) */
	rho = K * t * D * Nd2;
	break;
      case Callput::put:
	theta -= (r * K * D * Nmd2);
	/* rho: -K.t.exp(-r.t).N(-2) */
	rho = -K * t * D * Nmd2;
	break;
      }
	
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
		   xtag("N(d1)", Nd1),
		   xtag("N'(d1)", nd1),
		   xtag("N(d2)", Nd2),
		   xtag("tv", tv),
		   xtag("delta", delta),
		   xtag("gamma", gamma),
		   xtag("vega", vega),
		   xtag("theta", theta),
		   xtag("rho", rho)
		   );
      }

      return Greeks(tv, delta, gamma, vega, theta, rho);
    } /*call_greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end BlackScholes.cpp */
