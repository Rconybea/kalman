/* @file StrikeSetGreeksSimSource.hpp */

#pragma once

#include "reactor/SecondarySource.hpp"
#include "option/GreeksCallback.hpp"

namespace xo {
  namespace option {
    using StrikeSetGreeksSimSource = xo::reactor::SecondarySource<GreeksEvent>;
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetGreeksSimSource.hpp */
