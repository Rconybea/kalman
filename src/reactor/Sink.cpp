/* @file Sink.cpp */

#include "Sink.hpp"
#include "refcnt/Refcounted.hpp"

namespace xo {
  namespace reactor {
    ref::rp<SinkToConsole<std::pair<xo::time::utc_nanos, double>>>
    TemporaryTest::realization_printer()
    {
      return new SinkToConsole<std::pair<xo::time::utc_nanos, double>>();
    } /*realization_printer*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.cpp */
