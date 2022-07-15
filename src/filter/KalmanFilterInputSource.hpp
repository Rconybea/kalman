/* @file KalmanFilterInputSource.hpp */

#pragma once

#include "reactor/EventSource.hpp"
#include "filter/KalmanFilterInputCallback.hpp"

namespace xo {
  namespace kalman {
    /* Use:
     *    rp<KalmanFilterInputSource> src = ...;
     *    rp<KalmanFilterInputCallback> in_cb = ...;
     *
     *    src->add_callback(in_cb);
     * ...
     *    // src invokes in_cb->notify_input(
     *    src->remove_callback(in_cb);
     */
    using KalmanFilterInputSource = reactor::EventSource<
      /*KalmanFilterInput,*/
      KalmanFilterInputCallback
      /*&KalmanFilterInputCallback::notify_filter*/
      >;
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInputSource.hpp */
  
