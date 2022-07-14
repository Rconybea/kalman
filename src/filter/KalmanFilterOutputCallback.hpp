/* @file KalmanFilterOutputCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "filter/KalmanFilter.hpp"

namespace xo {
  namespace kalman {
    /* callback for consuming kalman filter output */
    class KalmanFilterOutputCallback : public ref::Refcount {
    public:
      /* notification with updated filter state */
      virtual void notify_filter(KalmanFilterStateExt const & state_ext) = 0;

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}
    }; /*KalmanFilterOutputCallback*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterOutputCallback.hpp */
