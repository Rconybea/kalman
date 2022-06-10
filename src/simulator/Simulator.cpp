/* @file Simulator.cpp */

#include "Simulator.hpp"
#include "time/Time.hpp"
#include <algorithm>

namespace xo {
  namespace sim {
    bool
    Simulator::is_source_present(SimulationSource * src) const
    {
      for(SimulationSource * s : this->src_v_) {
	if(s == src)
	  return true;
      }

      return false;
    } /*is_source_pesent*/

    time::utc_nanos
    Simulator::next_tm() const {
      if(this->sim_heap_.empty()) {
	/* 0 remaining events in simulator */
	return this->t0();
      }

      return this->sim_heap_.front().t0();
    } /*next_tm*/

    bool
    Simulator::add_source(SimulationSource * src)
    {
      if(!src || this->is_source_present(src))
	return false;

      src->advance_until(this->t0(), false /*!replay_flag*/);

      this->src_v_.push_back(src);

      if(src->is_exhausted()) {
	;
      } else {
	/* also add to simulation heap */
	this->sim_heap_.push_back(SourceTimestamp(src->current_tm(), src));

	/* use std::greater<> because we need a min-heap;
	 * smallest timestamp at the front
	 */
	std::push_heap(this->sim_heap_.begin(),
		       this->sim_heap_.end(),
		       std::greater<SourceTimestamp>());
      }

      return true;
    } /*add_source*/

    void
    Simulator::advance_one_event()
    {
      if(this->sim_heap_.empty()) {
	/* nothing todo */
	return;
      }

      /* *src is source with earliest timestamp */
      SimulationSource * src
	= this->sim_heap_.front().src();

      src->advance_one();

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

      if(src->is_exhausted()) {
	/* permanently remove src from .sim_heap */
	this->sim_heap_.pop_back();
      } else {
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
      }	
    } /*advance_one_event*/
  } /*namespace sim*/
} /*namespace xo*/

/* end Simulator.cpp */
