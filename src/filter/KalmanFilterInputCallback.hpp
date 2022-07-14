/* @file KalmanFilterInputCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "filter/KalmanFilter.hpp"

namespace xo {
  namespace kalman {
    /* callback for receiving kalman filter input */
    class KalmanFilterInputCallback : public ref::Refcount {
    public:
      /* notification with updated filter state */
      virtual void notify_input(KalmanFilterInput const & input) = 0;

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}
    }; /*KalmanFilterInputCallback*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputCallback.hpp */
