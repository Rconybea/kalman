/* @file GreeksCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "option/Greeks.hpp"

namespace xo {
  namespace option {
    /* callback for consuming option greeks */
    class GreeksCallback : public ref::Refcount {
    public:
      /* notification with bbo option tick */
      virtual void notify_greeks(GreeksEvent const & greeks) = 0;

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}
    }; /*GreeksCallback*/
   
    /* connect .notify_bbo() up to a std::function */
    class FunctionGreeksCb : public GreeksCallback {
    public:
      FunctionGreeksCb(std::function<void (GreeksEvent const &)> const & fn) : fn_(fn) {}

      // ----- inherited from GreeksCallback -----

      virtual void notify_greeks(GreeksEvent const & greeks) override;
      
    private:
      /* .notify_greeks() calls this function */
      std::function<void (GreeksEvent const &)> fn_;
    }; /*FunctionGreeksCb*/

  } /*namespace option*/
} /*namespace xo*/

/* end GreeksCallback.hpp */
