/* @file SimulationSource.hpp */

#pragma once

#include "queue/Source.hpp"
#include "time/Time.hpp"

namespace xo {
  namespace sim {
#ifdef OBSOLETE
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
      using Reactor = reactor::Reactor;
      using utc_nanos = xo::time::utc_nanos;

    public:
      // ----- inherited from reactor::Source -----

      virtual bool is_empty() const override = 0;
      virtual bool is_exhausted() const override = 0;
      /* if .is_exhausted = false:
       *   - next event time.
       *      more precisely:  no events exist in this source prior to .t0
       * otherwise not defined.
       */
      virtual utc_nanos sim_current_tm() const override = 0;
      /* release one event from this source.
       *
       * promise:
       * - new .current_tm() >= old .current_tm() || .is_exhausted()
       *
       * returns #of events actually released (0 or 1)
       */
      virtual std::uint64_t deliver_one() override = 0;
      /* promise:
       * - .current_tm() > tm || .is_exhausted() = true
       * - if replay_flag is true,   then any events between
       *   prev .t0() and new .t0() will have been published
       *
       * returns #of events delivered.
       * does not count events that were skipped, so always returns 0 if
       * replay_flag is false
       */
      virtual std::uint64_t sim_advance_until(utc_nanos tm, bool replay_flag) override = 0;
      virtual void notify_reactor_add(Reactor * /*reactor*/) override {}
      virtual void notify_reactor_remove(Reactor * /*reactor*/) override {}
    }; /*SimulationSource*/

    using SimulationSourcePtr = ref::rp<SimulationSource>;
#endif
  } /*namespace sim*/
} /*namespace xo*/

/* end SimulationSource.hpp */
