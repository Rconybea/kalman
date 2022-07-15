/* @file Source.cpp */

#include "reactor/ReactorSource.hpp"
#include <cstdint>

namespace xo {
  using xo::time::Time;
  using xo::time::utc_nanos;

  namespace reactor {
    utc_nanos
    ReactorSource::online_current_tm() const
    {
      /* for an online source:
       * .is_exhausted() must always be false;
       * this implies that .sim_current_tm() should
       * not be called in the first place
       */

      assert(false);

      return Time::epoch();
    } /*online_current_tm*/

    std::uint64_t
    ReactorSource::online_advance_until(utc_nanos /*tm*/,
					bool /*replay_flag*/)
    {
      return 0;
    } /*online_advance_until*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Source.cpp */
