/* @file Greeks.hpp */

#pragma once

namespace xo {
  namespace option {
    /* encapsulate result of greeks calculation for an option */
    class Greeks {
    public:
      Greeks() = default;
      Greeks(double tv, double delta, double gamma, double vega, double theta, double rho)
	: tv_{tv}, delta_{delta}, gamma_{gamma}, vega_{vega}, theta_{theta}, rho_{rho} {}

      double tv() const { return tv_; }
      double delta() const { return delta_; }
      double gamma() const { return gamma_; }
      double vega() const { return vega_; }
      double theta() const { return theta_; }
      double rho() const { return rho_; }

    private:
      /* theory price, in per-share units */
      double tv_ = 0.0;
      /* option delta;  dimensionless (always in [-1, +1]) */
      double delta_ = 0.0;
      /* option gamma;  per 100% change in spot */
      double gamma_ = 0.0;
      /* option vega;  per 100% change in volatility */
      double vega_ = 0.0;
      /* option theta;  change in value w.r.t elapsed time */
      double theta_ = 0.0;
      /* option rho;  change in value w.r.t interest rates */
      double rho_ = 0.0;
    }; /*Greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */
