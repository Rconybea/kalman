/* @file KalmanFilterInputCallback.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "filter/KalmanFilter.hpp"

namespace xo {
  namespace kalman {
    using KalmanFilterInputCallback = reactor::Sink1<KalmanFilterInput>;

#ifdef OBSOLETE
    class KalmanFilterInputCallback : public reactor::Sink1<KalmanFilterInput> {
    public:
      // ----- inherited from Sink1<KalmanFilterInput> -----

      /* notification with updated filter state */
      virtual void notify_input(KalmanFilterInput const & input) override = 0;

      /* CallbackSet invokes these on add/remove events */
      virtual void notify_add_callback() override {}
      virtual void notify_remove_callback() override {}
    }; /*KalmanFilterInputCallback*/
#endif
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputCallback.hpp */
