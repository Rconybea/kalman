/* @file Greeks.hpp */

namespace xo {
  namespace option {
    class Greeks {
    public:
      Greeks() = default;
      Greeks(double tv, double delta) : tv_{tv}, delta_{delta} {}

      double tv() const { return tv_; }
      double delta() const { return delta_; }

    private:
      /* theory price, in per-share units */
      double tv_ = 0.0;
      /* option delta;  dimensionless (always in [-1, +1]) */
      double delta_ = 0.0;
    }; /*Greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */
