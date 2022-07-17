/* @file StrikeSetOmdSimSource.hpp */

#pragma once

#include "reactor/SecondarySource.hpp"
#include "option/OmdCallback.hpp"

namespace xo {
  namespace option {
    using StrikeSetOmdSimSource = xo::reactor::SecondarySource<BboTick>;
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmdSimSource.hpp */
