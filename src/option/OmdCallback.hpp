/* @file OmdCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "option/BboTick.hpp"

namespace xo {
  namespace option {
    /* callback for consuming market data */
    class OmdCallback : public ref::Refcount {
    public:
      /* notification with bbo option tick */
      virtual void notify_bbo(BboTick const & bbo_tick) = 0;

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}
    }; /*OmdCallback*/
   
  } /*namespace option*/
} /*namespace xo*/

/* end OmdCallback.hpp */
