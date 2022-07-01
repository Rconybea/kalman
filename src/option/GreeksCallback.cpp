/* @file GreeksCallback.cpp */

#include "GreeksCallback.hpp"

namespace xo {
  namespace option {
    void
    FunctionGreeksCb::notify_greeks(GreeksEvent const & greeks)
    {
      this->fn_(greeks);
    } /*notify_greeks*/
  } /*namespace option*/
} /*namespace xo*/

/* end GreeksCallback.cpp */
