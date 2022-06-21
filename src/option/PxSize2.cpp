/* @file PxSize2.cpp */

#include "PxSize2.hpp"

namespace xo {
  namespace option {
    PxSize2
    PxSize2::with_size(Size z, Px2 const & px2)
    {
      return PxSize2(z, px2.bid_px(), px2.ask_px(), z);
    } /*with_size*/
  } /*namespace option*/
} /*namespace xo*/

/* end PxSize2.cpp */
