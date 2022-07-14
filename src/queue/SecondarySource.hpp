/* @file SecondarySource.hpp */

#pragma once

#include "time/Time.hpp"
#include "queue/EventSource.hpp"
#include "queue/Reactor.hpp"
#include "callback/CallbackSet.hpp"
#include <vector>

namespace xo {
  namespace reactor {
    /* a source that collects + forwards events produced
     * elsewhere.   Events will be forwarded to callbacks in timestamp order
     * by a running simulator
     *
     * For example StrikeSetMarketModel uses this to deliver omd and greeks events.
     *
     * Require:
     * - Event must be null-constructible
     * - Event must be copyable.
     *   events are copied as they are delivered
     * - Event must be comparable (will be stored in a heap);
     *   comparison must have property that if for two events ev1, ev2 we have
     *   ev1.tm() < ev2.tm(),   then it must also be the case that ev1 < ev2
     * - Event must have a method .tm() that supplies a timestamp
     *   for heap ordering
     * - Callback must be refcountable (i.e. ref::rp<Callback> is well-formed)
     * - MemberFn must identify a method in class Callback
     * - MemberFn signature must accept invocation like
     *     Callback * cb = ...;
     *     MemberFn member_fn = ...;
     *     Event const & evt = ...;
     *     (cb->*member_fn)(evt);
     */
    template<typename Event,
	     typename Callback,
	     void (Callback::*member_fn)(Event const &)>
    class SecondarySource : public EventSource<Callback> {
    public:
      using Reactor = reactor::Reactor;
      template<typename Fn>
      using CallbackSet = fn::CallbackSet<Fn>;
      using utc_nanos = xo::time::utc_nanos;
      using scope = logutil::scope;

    public:
      static ref::rp<SecondarySource> make() { return new SecondarySource(); }

      void notify_upstream_exhausted() { this->upstream_exhausted_ = true; }

      /* make event available to reactor,
       * by adding to .event_heap
       */
      void notify_event(Event const & ev) {
	using logutil::xtag;

	constexpr bool c_logging_enabled_flag = true;
	scope lscope("SecondarySource::notify_event", c_logging_enabled_flag);

	if(this->upstream_exhausted_) {
	  throw std::runtime_error("SecondarySource::notify_event"
				   ": not allowed after upstream exhausted");
	}

	if(this->current_tm_ < ev.tm())
	  this->current_tm_ = ev.tm();

	/* if heap is empty when .notify_event() begins,
	 * then reactor/simulator needs to be notified that source is no longer empty
	 */
	bool is_priming = this->event_heap_.empty();

	this->event_heap_.push_back(ev);

	/* restore heap property:
	 * use std::greater<> because we need min-heap, with smallest
	 * timestamp at the front
	 */
	std::push_heap(this->event_heap_.begin(),
		       this->event_heap_.end(),
		       std::greater<Event>());

	Reactor * r = this->parent_reactor_;

	if (c_logging_enabled_flag)
	  lscope.log("publish",
		     xtag("is-priming", is_priming),
		     xtag("reactor", r),
		     xtag("event_heap.size", this->event_heap_.size()),
		     xtag("event.tm", ev.tm()));

	if (is_priming && r) {
	  r->notify_source_primed(ref::brw<ReactorSource>::from_native(this));
	}
      } /*notify_event*/

      template<typename T>
      void notify_event_v(T const & v) {
	for(Event const & ev : v)
	  this->notify_event(ev);
      } /*notify_event_v*/

      // ----- inherited from reactor::EventSource -----

      void add_callback(ref::rp<Callback> const & cb) override {
	this->cb_set_.add_callback(cb);
      } /*add_callback*/

      void remove_callback(ref::rp<Callback> const & cb) override {
	this->cb_set_.remove_callback(cb);
      } /*remove_callback*/

      // ----- inherited from reactor::Source -----

      virtual bool is_empty() const override { return this->event_heap_.empty(); }
      virtual bool is_exhausted() const override { return this->upstream_exhausted_ && this->is_empty(); }      

      virtual utc_nanos sim_current_tm() const override {
	if(this->event_heap_.empty()) {
	  /* this is a tricky case.
	   * it means this source doesn't
	   * _know_ specific next event yet;  however new events
	   * may appear at any time by way of .notify_event()
	   *
	   * If event doesn't know next event,  then .current_tm isn't useful
	   * for establishing priority relative to other sources.
	   * rely on priming mechanism instead,
	   * which means that control should never come here
	   */

	  return this->current_tm_; 
	} else {
	  Event const & next_ev = this->event_heap_.front();

	  return next_ev.tm();
	}
      } /*sim_current_tm*/

      virtual std::uint64_t deliver_one() override { return this->deliver_one_aux(true /*replay_flag*/); }

      virtual std::uint64_t sim_advance_until(utc_nanos target_tm,
					      bool replay_flag) override
      {
	uint64_t retval;

	while(!this->event_heap_.empty()) {
	  utc_nanos tm = this->sim_current_tm();

	  if(tm < target_tm) {
	    retval += this->deliver_one_aux(replay_flag);
	  } else {
	    break;
	  }
	}

	return retval;
      } /*advance_until*/

      virtual void notify_reactor_add(Reactor * reactor) override {
	assert(!this->parent_reactor_);

	this->parent_reactor_ = reactor;
      } /*notify_reactor_add*/

      virtual void notify_reactor_remove(Reactor * /*reactor*/) override {}

    private:
      /* deliver one event from .event_heap[];   invoke callback for that event
       * provided replay_flag is true.
       * the event is removed from .event_heap[],  enforcing that any single event
       * can be delivered once.
       */
      std::uint64_t deliver_one_aux(bool replay_flag) {
	if (event_heap_.empty()) {
	  return 0;
	}

	/* need to remove event _before_ invoking callbacks.
	 * callbacks may trigger activity that re-entrantly modifies .event_heap[]
	 */
	Event ev = this->event_heap_.front();

	/* move just-consumed item to the back of the heap */
	std::pop_heap(this->event_heap_.begin(),
		      this->event_heap_.end(),
		      std::greater<Event>());

	this->event_heap_.pop_back();

	if(replay_flag) {
	  /* publish first event */
	  this->cb_set_.invoke(member_fn, ev);
	}

	return 1;
      } /*deliver_one_aux*/

    private:
      /* current time for this source */
      utc_nanos current_tm_;

      /* may set this to true, just once, to announce that upstream
       * will send no more events.
       * see .notify_upstream_exhausted()
       */
      bool upstream_exhausted_ = false;

      /* queued events to be delivered to reactor */
      std::vector<Event> event_heap_;

      /* reactor/simulator being used to schedule consumption from this source */
      Reactor * parent_reactor_ = nullptr;

      /* invoke callbacks in this set for each event */
      CallbackSet<ref::rp<Callback>> cb_set_;
    }; /*SecondarySource*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end SecondarySource.hpp */
