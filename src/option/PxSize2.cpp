/* @file PxSize2.cpp */

#include "PxSize2.hpp"

namespace xo {
  namespace option {
    PxSize2
    PxSize2::with_size(Size z, Px2 const & px2)
    {
      return PxSize2(z, px2.bid_px(), px2.ask_px(), z);
    } /*with_size*/

    void
    PxSize2::display(std::ostream & os) const
    {
      /* e.g.
       *   {pxz2 1x 1.20-1.25 x1}
       */

      os << "{pxz2 "
	 << bid_sz_ << "x "
	 << Px2(bid_px_, ask_px_)
	 << " x" << ask_sz_
	 << "}";
    } /*display*/
  } /*namespace option*/
} /*namespace xo*/

/* end PxSize2.cpp */
