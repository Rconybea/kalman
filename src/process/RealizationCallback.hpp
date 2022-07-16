/* @file RealizationCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "time/Time.hpp"
#include <utility>

namespace xo {
  namespace process {
    /* callback for consuming stochastic process realizations */
    template<typename T>
    class RealizationCallback : public ref::Refcount {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      /* notification with process event (std::pair<utc_nanos, T>)
       * see StochasticProcess<T>::event_type
       */
      virtual void notify_ev(std::pair<utc_nanos, T> const & ev);

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}
    }; /*RealizationCallback*/
  } /*namespace process*/
} /*namespace xo*/

/* end RealizationCallback.hpp */
