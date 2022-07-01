/* @file PxSize2.hpp */

#pragma once

#include "option_util/Px2.hpp"
#include "option_util/Size.hpp"

namespace xo {
  namespace option {
    /* a bid/ask pair with size */
    class PxSize2 {
    public:
      PxSize2() = default;
      PxSize2(Size bid_sz, Price bid_px, Price ask_px, Size ask_sz)
	: bid_sz_{bid_sz}, bid_px_{bid_px}, ask_px_{ask_px}, ask_sz_{ask_sz} {}

      static PxSize2 with_size(Size z, Px2 const & px2);

      Size bid_sz() const { return bid_sz_; }
      Price bid_px() const { return bid_px_; }
      Price ask_px() const { return ask_px_; }
      Size ask_sz() const { return ask_sz_; }

      bool is_bid_present() const { return bid_sz_.is_valid(); }
      bool is_ask_present() const { return ask_sz_.is_valid(); }

      /* e.g.
       *   PxSize2 p(1, 1.2, 1.25, 3);
       *   p.display(os)
       * writes
       *   {pxz2 1x 1.20-1.25 x3}
       * on stream os
       */
      void display(std::ostream & os) const;

    private:
      Size bid_sz_;
      Price bid_px_;
      Price ask_px_;
      Size ask_sz_;
    }; /*PxSize2*/

    inline std::ostream &
    operator<<(std::ostream & os, PxSize2 const & x) {
      x.display(os);
      return os;
    } /*operator<<*/
  } /*namespace option*/
} /*namespace xo*/

  /* end PxSize2.hpp */
  
