/* @file OmdCallback.cpp */

#include "OmdCallback.hpp"

namespace xo {
  namespace option {
    void
    FunctionOmdCb::notify_bbo(BboTick const & bbo_tick)
    {
      this->fn_(bbo_tick);
    } /*notify_bbo*/
  } /*namespace option*/
} /*namespace xo*/

/* end OmdCallback.cpp */
