/* @file SimulationSource.hpp */

#pragma once

#include "queue/Source.hpp"
#include "time/Time.hpp"

namespace xo {
  namespace sim {
    /* a simulation source has:
     *
     * - current time t0.
     * - advance_until(t) method.
     *   a simulation source is responsible for its own event
     *   representation,  and negotiation with event sinks.
     *
     * loop for consuming a simulation source:
     *
     *   SimulationSource * s = ...;
     *   while(!s->is_exhausted()) {
     *      utc_nanos tm = s->t0();
     *
     *      s->advance_until(tm, true);
     *   }
     */
    class SimulationSource : public reactor::Source {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* true when all events from this sim source
       * have been replayed
       */
      virtual bool is_exhausted() const = 0;

      /* if .is_exhausted = false:
       *   - next event time.
       *      more precisely:  no events exist in this source prior to .t0
       * otherwise not defined.
       */
      virtual utc_nanos current_tm() const = 0;

      /* promise:
       * - .t0() > tm || .is_exhausted() = true
       * - if replay_flag is true,   then any events between
       *   prev .t0() and new .t0() will have been published
       */
      virtual void advance_until(utc_nanos tm, bool replay_flag) = 0;

      /* release one event from this source.
       *
       * promise:
       * - new .t0() >= old .t0() || .is_exhausted()
       *
       * returns #of events actually released (0 or 1)
       */
      virtual std::uint64_t advance_one() = 0;

      // ----- inherited from reactor::Source -----

      virtual bool is_empty() const override { return this->is_exhausted(); }
      virtual std::uint64_t deliver_one() override { return this->advance_one(); }
    }; /*SimulationSource*/
  } /*namespace sim*/
} /*namespace xo*/

/* end SimulationSource.hpp */
