/* @file Greeks.hpp */

namespace xo {
  namespace option {
    class Greeks {
    public:
      Greeks() = default;
      Greeks(double tv, double delta, double gamma)
	: tv_{tv}, delta_{delta}, gamma_(gamma) {}

      double tv() const { return tv_; }
      double delta() const { return delta_; }
      double gamma() const { return gamma_; }

    private:
      /* theory price, in per-share units */
      double tv_ = 0.0;
      /* option delta;  dimensionless (always in [-1, +1]) */
      double delta_ = 0.0;
      /* option gamma;  per 100% change in spot */
      double gamma_ = 0.0;
    }; /*Greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */