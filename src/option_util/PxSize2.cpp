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
	 << sz_v_[side2int(Side::bid)] << "x "
	 << px2_
	 << " x" << sz_v_[side2int(Side::ask)]
	 << "}";
    } /*display*/

    void
    PxSize2::assign_pxz(Side s,
			PxSize2 const & pxz2)
    {
      this->sz_v_[side2int(s)] = pxz2.size(s);
      this->px2_.assign_px(s, pxz2.px(s));
    } /*set_side*/
  } /*namespace option*/
} /*namespace xo*/

/* end PxSize2.cpp */
