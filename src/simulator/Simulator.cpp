/* @file Simulator.cpp */

#include "Simulator.hpp"
#include "time/Time.hpp"
#include "logutil/scope.hpp"
#include <_types/_uint64_t.h>
#include <algorithm>

namespace xo {
  using xo::reactor::Source;
  using xo::ref::brw;
  using xo::time::utc_nanos;
  using logutil::scope;
  using logutil::xtag;

  namespace sim {
    Simulator::~Simulator() {
      constexpr char const * c_self = "Simulator::dtor";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

      if(c_logging_enabled)
	lscope.log(c_self, ": clear heap ..");

      this->sim_heap_.clear();
      
      if(c_logging_enabled) {
	lscope.log(c_self, ": visit .src_v", xtag("size", this->src_v_.size()));
	for(size_t i=0; i<this->src_v_.size(); ++i) {
	  lscope.log(c_self, ":src_v[", i, "] ", this->src_v_[i].get());
	}
      }

      if(c_logging_enabled)
	lscope.log(c_self, ": clear .src_v", xtag("size", this->src_v_.size()));
      this->src_v_.clear();

    } /*dtor*/

    bool
    Simulator::is_source_present(brw<SimulationSource> src) const
    {
      for(SimulationSourcePtr const & s : this->src_v_) {
	if(s == src)
	  return true;
      }

      return false;
    } /*is_source_pesent*/

    utc_nanos
    Simulator::next_tm() const {
      if(this->sim_heap_.empty()) {
	/* 0 remaining events in simulator */
	return this->t0();
      }

      return this->sim_heap_.front().t0();
    } /*next_tm*/

    void
    Simulator::notify_source_primed(brw<Source> src)
    {
      brw<SimulationSource> sim_src = brw<SimulationSource>::from(src);

      if(!sim_src)
	return;

      /* inform Simulator when a source transitions from
       * 'notready' to 'ready'.
       *
       * this means:
       * - source knows its next event
       * - source should be put back into .sim_heap
       */
      this->heap_insert_source(sim_src.get());
    } /*notify_source_primed*/

    bool
    Simulator::add_source(brw<Source> src)
    {
      constexpr char const * c_self = "Simulator::add_source";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

      if(c_logging_enabled)
	lscope.log(c_self, ": enter", xtag("src", src.get()));

      /* verify that src isa SimulationSource instance.
       * Simulator does not support non-simulation sources.
       */
      brw<SimulationSource> sim_src = brw<SimulationSource>::from(src);

      if(!sim_src || this->is_source_present(sim_src))
	return false;

      sim_src->advance_until(this->t0(), false /*!replay_flag*/);

      this->src_v_.push_back(sim_src.promote());

      if(sim_src->is_exhausted()) {
	;
      } else {
	/* also add to simulation heap */
	this->sim_heap_.push_back(SourceTimestamp(sim_src->current_tm(),
						  sim_src.get()));

	/* use std::greater<> because we need a min-heap;
	 * smallest timestamp at the front
	 */
	std::push_heap(this->sim_heap_.begin(),
		       this->sim_heap_.end(),
		       std::greater<SourceTimestamp>());
      }

      return true;
    } /*add_source*/

    bool
    Simulator::remove_source(brw<Source> src)
    {
      //constexpr char const * c_self = "Simulator::remove_source";

      brw<SimulationSource> sim_src = brw<SimulationSource>::from(src);

      if(!sim_src || !this->is_source_present(sim_src))
	return false;

      /* WARNING: O(n)implementation here */

      /* rebuild .sim_heap,  with sim_src removed */
      std::vector<SourceTimestamp> sim_heap2;

      for(SourceTimestamp const & item : this->sim_heap_) {
	if(item.src() == sim_src.get()) {
	  /* item refers to the source we are removing -> discard */
	  ;
	} else {
	  sim_heap2.push_back(item);

	  std::push_heap(sim_heap2.begin(),
			 sim_heap2.end(),
			 std::greater<SourceTimestamp>());
	}
      }

      /* now discard .sim_heap,  replacing with sim_heap2 */
      this->sim_heap_ = std::move(sim_heap2);

      return true;
    } /*remove_source*/

    std::uint64_t
    Simulator::run_one() {
      return this->advance_one_event();
    } /*run_one*/

    void
    Simulator::heap_insert_source(SimulationSource * src)
    {
      std::size_t simheap_z
	= this->sim_heap_.size();

      /* re-insert at new timestamp */
      this->sim_heap_[simheap_z - 1]
	= SourceTimestamp(src->current_tm(), src);

      /* use std::greater<> because we need a min-heap;
       * smallest timestamp at the front
       */
      std::push_heap(this->sim_heap_.begin(),
		     this->sim_heap_.end(),
		     std::greater<SourceTimestamp>());
    } /*heap_insert_source*/

    std::uint64_t
    Simulator::advance_one_event()
    {
      if(this->sim_heap_.empty()) {
	/* nothing todo */
	return 0;
      }

      /* *src is source with earliest timestamp */
      SimulationSource * src
	= this->sim_heap_.front().src();

      uint64_t retval = src->advance_one();

      /* src.t0 may have advanced */

      /* moves just-consumed timestamp event for src
       * to back of .sim_heap
       */
      std::pop_heap(this->sim_heap_.begin(),
		    this->sim_heap_.end(),
		    std::greater<SourceTimestamp>());

      /* now .sim_heap[.sim_heap.size() = 1].src() is src,
       * with stale timestamp 
       */

      if(src->is_exhausted() || src->is_notprimed()) {
	/* remove src from .sim_
	 * - if src->is_exhausted(),  permanently
	 * - if src->is_notready(),  until source calls
	 *   .notify_source_ready()
	 */
	this->sim_heap_.pop_back();
      } else {
	this->heap_insert_source(src);
      }

      return retval;
    } /*advance_one_event*/

    void
    Simulator::run_until(utc_nanos t1)
    {
      while(!this->is_exhausted()) {
	utc_nanos t = this->next_tm();

	if(t > t1)
	  break;

	this->advance_one_event();
      } /*loop until done*/
    } /*run_until*/
  } /*namespace sim*/
} /*namespace xo*/

/* end Simulator.cpp */
