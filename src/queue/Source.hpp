/* @file Source.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include <cstdint>

namespace xo {
  namespace reactor {
    class Reactor;

    /* abstract api for a source of events.
     * Event representation is left open:  Sources and Sinks
     * need to have compatible event representations,
     * and coordination is left to such (Source, Sink) pairs.
     *
     * Source->Sink activity is expected to be mediated by a reactor,
     * that implements the Reactor api.
     *
     * At any time,  A Source can be associated with at most one reactor.
     * Sources are informed of Reactor<->Source association being
     * formed/broken by the 
     *   .notify_reactor_add(), .notify_reactor_remove()
     * methods
     */
    class Source : public ref::Refcount {
    public:
      virtual ~Source() = default;

      /* true if source is currently empty (has 0 events to deliver) */
      virtual bool is_empty() const = 0;
      bool is_nonempty() const { return !this->is_empty(); }

      /* true when source doesn't know its next event
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

      /* deliver one  event to attached sink
       * interpretation of 'one event' is source-specific;
       * could be a collapsed or batched event in practice.
       *
       * no-op if source is empty
       *
       * returns #of events delivered.  Must be 0 or 1 in this context
       */
      virtual std::uint64_t deliver_one() = 0;

      /* informs source when it's added to a reactor
       * (see Reactor.add_source())
       */
      virtual void notify_reactor_add(Reactor * /*reactor*/) {}

      /* informs source when it's removed from a reactor
       * (see Reactor.remove_source())
       */
      virtual void notify_reactor_remove(Reactor * /*reactor*/) {}
    }; /*Source*/

    using SourcePtr = ref::rp<Source>;
  } /*namespace reactor*/
} /*namespace xo*/

/* end Source.hpp */
