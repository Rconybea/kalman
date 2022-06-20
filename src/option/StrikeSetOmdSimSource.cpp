/* @file StrikeSetOmdSimSource.cpp */

#include "StrikeSetOmdSimSource.hpp"
#include "queue/Reactor.hpp"
#include "time/Time.hpp"
#include <cstdint>

namespace xo {
  using reactor::Source;
  using ref::brw;
  using logutil::xtag;

  namespace option {
    void
    StrikeSetOmdSimSource::notify_upstream_exhausted()
    {
      this->upstream_exhausted_ = true;
    } /*notify_upstream_exhausted*/

    void
    StrikeSetOmdSimSource::notify_bbo(BboTick const & tick)
    {
      if(this->upstream_exhausted_) {
	throw std::runtime_error("StrikeSetOmdSimSource::notify_bbo"
				 ": not allowed after upstream exhausted");
      }

      if(this->current_tm_ < tick.tm())
	this->current_tm_ = tick.tm();

      bool is_priming = this->omd_heap_.empty();

      this->omd_heap_.push_back(tick);

      /* restore heap property:
       * use std::greater<> because we need min-heap, with smallest
       * timestamp at the front
       */
      std::push_heap(this->omd_heap_.begin(),
		     this->omd_heap_.end(),
		     std::greater<BboTick>());

      Reactor * r = this->parent_reactor_;

      if (is_priming && r) {
        r->notify_source_primed(brw<Source>::from_native(this));
      }
    } /*notify_bbo*/

    bool
    StrikeSetOmdSimSource::is_exhausted() const
    {
      return (this->upstream_exhausted_
	      && this->omd_heap_.empty());
    } /*is_exhausted*/

    bool
    StrikeSetOmdSimSource::is_empty() const
    {
      return this->omd_heap_.empty();
    } /*is_empty*/

    time::utc_nanos
    StrikeSetOmdSimSource::current_tm() const
    {
      if(this->omd_heap_.empty()) {
	/* this is a tricky case.
	 * it means StrikeSetOmdSimSource doesn't
	 * _know_ specific next event yet;  however new events
	 * may appear as of next call to .notify_bbo()
	 *
	 * DESIGN:
	 * best thing maybe:
	 * - allow not-exhausted sim source to be temporarily removed from
	 *   simulator heap
	 * - put source back into heap as soon as a new event appears.
	 *   to do that:
	 *   - source will need to remember simulator
	 *     (see notify_reactor_add() / notify_reactor_remove())
	 *   - need reactor: notify_source() method,  for when previously empty
	 *     source changes to non-empty
	 */

	return current_tm_; 
      } else {
	BboTick const & next_tick = this->omd_heap_.front();

	return next_tick.tm();
      }
    } /*current_tm*/

    /* TODO: return #of events published */
    void
    StrikeSetOmdSimSource::advance_until(utc_nanos target_tm,
					 bool replay_flag)
    {
      while(!this->omd_heap_.empty()) {
	utc_nanos tm = this->current_tm();

	if(tm < target_tm) {
	  this->advance_one_aux(replay_flag);
	} else {
	  break;
	}
      }

#ifdef NOT_IN_USE
      if(this->current_tm() < target_tm) {
	/* TODO:  if preparing source before starting sim,
	 *        this corner case is harmless.
	 *        Can detect by observing if simulator is attached.
	 *        if simulator is attached,   then need to absent this
	 *        source from simulator heap while it has no events
	 */

	throw std::runtime_error(tostr("StrikeSetOmdSimSource: cannot .advance_until()"
				       " through empty event heap",
				       xtag("target_tm", target_tm),
				       xtag("replay_flag", replay_flag)));
      }
#endif
    } /*advance_until*/

    std::uint64_t
    StrikeSetOmdSimSource::advance_one_aux(bool replay_flag)
    {
      if (omd_heap_.empty()) {
	return 0;
      }

      BboTick bbo_tick
	= this->omd_heap_.front();

      /* move just-consumed item to the back of the heap */
      std::pop_heap(this->omd_heap_.begin(),
		    this->omd_heap_.end(),
		    std::greater<BboTick>());

      this->omd_heap_.pop_back();

      if(replay_flag) {
	/* publish bbo_tick */
	XO_STUB();
      }

      return 1;
    } /*advance_one_aux*/

    std::uint64_t
    StrikeSetOmdSimSource::advance_one()
    {
      return this->advance_one_aux(true /*replay_flag*/);
    } /*advance_one*/

    void
    StrikeSetOmdSimSource::notify_reactor_add(Reactor * reactor)
    {
      assert(!this->parent_reactor_);

      this->parent_reactor_ = reactor;
    } /*notify_reactor_add*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmdSimSource.cpp */
