/* @file KalmanFilterOutputCallback.hpp */

#pragma once

#include "reactor/Sink.hpp"
#include "filter/KalmanFilter.hpp"

namespace xo {
  namespace kalman {
    /* callback for consuming kalman filter output */
    using KalmanFilterOutputCallback = reactor::Sink1<KalmanFilterStateExt>;
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterOutputCallback.hpp */
