/* @file StrikeSetGreeksSimSource.hpp */

#pragma once

#include "simulator/SecondarySimSource.hpp"
#include "option/GreeksCallback.hpp"

namespace xo {
  namespace option {
    using StrikeSetGreeksSimSource = xo::sim::SecondarySimSource<GreeksEvent, GreeksCallback, &GreeksCallback::notify_greeks>;
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetGreeksSimSource.hpp */
