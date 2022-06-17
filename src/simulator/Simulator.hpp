/* @file Simulator.hpp */

#pragma once

#include "queue/Reactor.hpp"
#include "simulator/SimulationSource.hpp"
#include "simulator/SourceTimestamp.hpp"
#include "refcnt/Refcounted.hpp"
#include "time/Time.hpp"
#include <vector>

namespace xo {
  namespace sim {

    /* Generic simulator
     *
     * - time advances monotonically
     * - applies a modifiable set of sources
     *
     * A Simulator isn't an example of a Reactor,
     * because it can't work with arbitrary Sources
     * (may find it expedient to fake this later,
     * so we can easily adopt
     *    Source.notify_reactor_add() / Source.notify_reactor_remove())
     * in a simulation context
     */
    class Simulator : public reactor::Reactor {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      ~Simulator();
      
      static ref::rp<Simulator> make(utc_nanos t0) { return new Simulator(t0); }

      /* value of .t0() is estabished in ctor.
       * it will not change except across call to .advance_one()
       * in particular .add_source() does not change .t0()
       */
      utc_nanos t0() const { return t0_; }

      /* true iff all simulation source are exhausted
       * a newly-created simulator is in the exhausted state;
       * it may transition to non-exhausted state across
       * call to .add_source()
       */
      bool is_exhausted() const { return this->src_v_.empty(); }

      /* true iff src has been added to this simulator
       * (by .add_source())
       */
      bool is_source_present(ref::brw<SimulationSource> src) const;

      /* promise:
       *   .next_tm() > .t0() || .is_exhausted()
       *
       * .next_tm() may decrease across .add_source() call
       * .next_tm() may increase across .advance_one() call
       */
      utc_nanos next_tm() const;

      /* emit the first available event from a single simulation source.
       * resolve ties arbitrarily.
       *
       * returns the #of events dispatched
       */
      std::uint64_t advance_one_event();

      /* run simulation until earliest event time t satisfies t > t1 */
      void run_until(utc_nanos t1);

      // ----- inherited from Reactor -----

      /* add a new simulation source.
       * event that precede .t0 will be discarded.
       *
       * returns true if src added;  false if already present
       */
      virtual bool add_source(ref::brw<reactor::Source> src) override;

      /* remove simulation source.
       * returns true if src removed;  false if was not present
       *
       * (not typically needed for simulations)
       */
      virtual bool remove_source(ref::brw<reactor::Source> src) override;

      /* synonym for .advance_one_event() */
      virtual std::uint64_t run_one() override;

    private:
      explicit Simulator(utc_nanos t0) : t0_(t0) {}

    private:
      /* simulation heap:
       * each unexhausted source appears
       * exactly once,  in increasing time order of next event
       *
       * Invariant:
       * - all sources s in .sim_heap satisfy:
       *   - s.is_exhausted() = false
       *   - s.t0() >= .t0
       */
      std::vector<SourceTimestamp> sim_heap_;

      /* current simulation clock */
      utc_nanos t0_;

      /* simulation sources
       * Invariant:
       * - all source s in .src_v satisfy:
       *   EITHER
       *     1.  s.is_exhausted() = true
       *   OR
       *     2.1 s.is_exhausted() = false
       *     2.2 s.t0() >= .t0
       */
      std::vector<SimulationSourcePtr> src_v_;
    }; /*Simulator*/

  } /*namespace sim*/
} /*namespace xo*/

/* end Simulator.hpp */
