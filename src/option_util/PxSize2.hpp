/* @file PxSize2.hpp */

#pragma once

#include "option_util/Px2.hpp"
#include "option_util/Size.hpp"
#include <array>

namespace xo {
  namespace option {
    /* a bid/ask pair with size */
    class PxSize2 {
    public:
      PxSize2() = default;
      PxSize2(Size bid_sz, Price bid_px, Price ask_px, Size ask_sz)
	: sz_v_{bid_sz, ask_sz}, px2_{bid_px, ask_px} {}

      static PxSize2 with_size(Size z, Px2 const & px2);

      Size bid_sz() const { return sz_v_[side2int(Side::bid)]; }
      Price bid_px() const { return px2_.px(Side::bid); }
      Price ask_px() const { return px2_.px(Side::ask); }
      Size ask_sz() const { return sz_v_[side2int(Side::ask)]; }

      Size size(Side s) const { return sz_v_[side2int(s)]; }
      Price px(Side s) const { return px2_.px(s); }

      bool is_side_present(Side s) const { return sz_v_[side2int(s)].is_valid(); }

      bool is_bid_present() const { return sz_v_[side2int(Side::bid)].is_valid(); }
      bool is_ask_present() const { return sz_v_[side2int(Side::ask)].is_valid(); }

      /* e.g.
       *   PxSize2 p(1, 1.2, 1.25, 3);
       *   p.display(os)
       * writes
       *   {pxz2 1x 1.20-1.25 x3}
       * on stream os
       */
      void display(std::ostream & os) const;

      /* set state for side s from corresponding fields in pxz2 */
      void assign_pxz(Side s, PxSize2 const & pxz2);
      
    private:
      /* bid,ask size */
      std::array<Size, 2> sz_v_;
      /* bid,ask price */
      Px2 px2_;
    }; /*PxSize2*/

    inline std::ostream &
    operator<<(std::ostream & os, PxSize2 const & x) {
      x.display(os);
      return os;
    } /*operator<<*/
  } /*namespace option*/
} /*namespace xo*/

/* end PxSize2.hpp */
  
