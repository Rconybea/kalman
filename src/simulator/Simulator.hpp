/* @file Simulator.hpp */

#pragma once

#include "simulator/SimulationSource.hpp"
#include "simulator/SourceTimestamp.hpp"
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
    class Simulator {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      explicit Simulator(utc_nanos t0) : t0_(t0) {}
      ~Simulator();
      
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
      bool is_source_present(refcnt::brw<SimulationSource> src) const;

      /* promise:
       *   .next_tm() > .t0() || .is_exhausted()
       *
       * .next_tm() may decrease across .add_source() call
       * .next_tm() may increase across .advance_one() call
       */
      utc_nanos next_tm() const;

      /* add a new simulation source.
       * event that precede .t0 will be discarded.
       *
       * returns true if src added;  false if already present
       */
      bool add_source(refcnt::brw<SimulationSource> src);

      /* emit the first available event from a single simulation source.
       * resolve ties arbitrarily
       */
      void advance_one_event();

      /* run simulation until earliest event time t satisfies t > t1 */
      void run_until(utc_nanos t1);

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
