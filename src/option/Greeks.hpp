/* @file Greeks.hpp */

namespace xo {
  namespace option {
    class Greeks {
    public:
      Greeks() = default;
      explicit Greeks(double tv) : tv_{tv} {}

      double tv() const { return tv_; }

    private:
      /* theory price, in per-share units */
      double tv_ = 0.0;
    }; /*Greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */
