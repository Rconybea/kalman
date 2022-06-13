/* @file Greeks.hpp */

namespace xo {
  namespace option {
    class Greeks {
    public:
      Greeks() = default;
      Greeks(double tv, double delta, double gamma, double vega, double theta)
	: tv_{tv}, delta_{delta}, gamma_{gamma}, vega_{vega}, theta_{theta} {}

      double tv() const { return tv_; }
      double delta() const { return delta_; }
      double gamma() const { return gamma_; }
      double vega() const { return vega_; }
      double theta() const { return theta_; }

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
    }; /*Greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */
