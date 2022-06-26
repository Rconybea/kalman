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
   
    /* connect .notify_bbo() up to a std::function */
    class FunctionOmdCb : public OmdCallback {
    public:
      FunctionOmdCb(std::function<void (BboTick const &)> const & fn) : fn_(fn) {}

      // ----- inherited from OmdCallback -----

      virtual void notify_bbo(BboTick const & bbo_tick) override;
      
    private:
      /* .notify_bbo() call this function */
      std::function<void (BboTick const &)> fn_;
    }; /*FunctionOmdCb*/

  } /*namespace option*/
} /*namespace xo*/

/* end OmdCallback.hpp */
