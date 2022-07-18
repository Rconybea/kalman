/* @file ReactorSource.hpp */

#pragma once

#include "reactor/Source.hpp"
#include "time/Time.hpp"
#include <cstdint>

namespace xo {
  namespace reactor {
    class Reactor;

    /* abstract api for a source of events.
     * Event representation is left open:  Sources and Sinks
     * need to have compatible event representations,
     * and coordination is left to such (Source, Sink) pairs.
     *
     * Source->Sink activity may be expected to be mediated by a reactor,
     * that implements the Reactor api.
     *
     * At any time,  A Source can be associated with at most one reactor.
     * Sources are informed of Reactor<->Source association being
     * formed/broken by the 
     *   .notify_reactor_add(), .notify_reactor_remove()
     * methods
     *
     * The source api intends also to provide for simulation.
     * There introduces two simulation-specific methods:
     *   .sim_current_tm()
     *   .sim_advance_until()
     *
     * A non-simulation source can implement these as calls to
     * .online_current_tm(), .online_advance_until() respectively
     *   .online_current_tm() aborts since an online source is never exhausted
     *   .online_advance_until() is a no-op that returns 0
     *
     * Loop for consuming from a primary simulation source:
     *
     *   brw<Source> s = ...;
     *   while(!s->is_exhausted())
     *     s->deliver_one();
     *
     * Secondary sources (sources that depend on other sources) can be
     * in a state where they don't know their next event,  in which case:
     *
     *   s->is_notprimed() == true
     */
    class ReactorSource : public AbstractSource {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      virtual ~ReactorSource() = default;

      /* true if source is currently empty (has 0 events to deliver) */
      virtual bool is_empty() const = 0;
      bool is_nonempty() const { return !this->is_empty(); }

      /* true when source knows its next event
       * A source that isn't primed is also excluded from simulation
       * heap until it becomes primed.
       * This make feasible simulation sources that
       * depend on other simulation sources
       */
      virtual bool is_primed() const { return !this->is_empty(); }
      virtual bool is_notprimed() const { return this->is_empty(); }

      /* if true, this source has no events,  and will never publish more events
       * - for sim,  return true for a standalone source that has replayed all events
       * - for rt,  set during orderly 
       */
      virtual bool is_exhausted() const = 0;

      /* if this is a simulation source and .is_exhausted is false:
       * returns next event time;  more precisely, no events exist prior to
       * this time.
       *
       * if sim, and .is_primed = true,
       * returns timestamp of next event
       */
      virtual utc_nanos sim_current_tm() const = 0;

      /* promise:
       * - .current_tm() > tm || .is_notprimed() || .is_exhausted() = true
       * - if replay_flag is true,  then any events between previous .current_tm()
       *    and new .current_tm() will have been published
       *
       * returns #of events delivered.
       * does not count events that were skipped,  so always returns 0 if
       * replay_flag is false
       */
      virtual std::uint64_t sim_advance_until(utc_nanos tm, bool replay_flag) = 0;

      /* informs source when it's added to a reactor
       * (see Reactor.add_source())
       */
      virtual void notify_reactor_add(Reactor * /*reactor*/) {}

      /* informs source when it's removed from a reactor
       * (see Reactor.remove_source())
       */
      virtual void notify_reactor_remove(Reactor * /*reactor*/) {}

      // ----- Inherited from AbstractSource -----

      /* deliver one  event to attached sink
       * interpretation of 'one event' is source-specific;
       * could be a collapsed or batched event in practice.
       *
       * no-op if source is empty.
       *
       * if sim, promise:
       * - new .current_tm >= old .current_tm() || .is_notprimed() || .is_exhausted()
       *
       * returns #of events delivered.  Must be 0 or 1 in this context
       */
      virtual std::uint64_t deliver_one() override = 0;

    protected:
      /* default implementations for online sources */
      utc_nanos online_current_tm() const;
      uint64_t online_advance_until(utc_nanos tm, bool replay_flag);
    }; /*ReactorSource*/

    using ReactorSourcePtr = ref::rp<ReactorSource>;
  } /*namespace reactor*/
} /*namespace xo*/

/* end ReactorSource.hpp */
