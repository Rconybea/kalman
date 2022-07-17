/* @file RealizationSimSource.hpp */

#pragma once

#include "reactor/ReactorSource.hpp"
#include "process/RealizationTracer.hpp"
#include "process/RealizationCallback.hpp"
#include "callback/CallbackSet.hpp"
#include <logutil/scope.hpp>
#include <functional>

namespace xo {
  namespace process {
    /* use a discrete realization of a continuous stochastic process,
     * as a simulation source.
     *
     * 1. Realization is developed lazily,  (see RealizationTracer<T>)
     * 2. Use a fixed discretization interval to develop realization
     * 3. events are consumed by Sink
     *
     * Require:
     * - EventSink.notify_source_exhausted()
     * - invoke EventSink(std::pair<utc_nanos, T>)
     */
    template <typename T, typename EventSink>
    class RealizationSourceBase : public xo::reactor::ReactorSource {
    public:
      using nanos = xo::time::nanos;

    public:
      ~RealizationSourceBase() {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "RealizationSimSource<>::dtor";
	constexpr bool c_logging_enabled = false;
	
	scope lscope(c_self, c_logging_enabled);
	if(c_logging_enabled)
	  lscope.log("delete instance", xtag("p", this));
      } /*dtor*/

      static ref::rp<RealizationSourceBase> make(ref::rp<RealizationTracer<T>> const & tracer,
						 nanos ev_interval_dt,
						 EventSink const & ev_sink)
      {
	using logutil::scope;
	using logutil::xtag;

	constexpr bool c_logging_enabled = false;

	auto p = new RealizationSourceBase(tracer, ev_interval_dt, ev_sink);

	scope lscope(sc_self_type, "::make", c_logging_enabled);
	if(c_logging_enabled)
	  lscope.log("create instance",
		     xtag("p", p),
		     xtag("bytes", sizeof(RealizationSourceBase)));

	return p;
      } /*make*/

#ifdef NOT_IN_USE
      static ref::rp<RealizationSimSource> make(ref::rp<RealizationTracer<T>> tracer,
						nanos ev_interval_dt,
						EventSink && ev_sink)
      {
	return new RealizationSimSource(tracer, ev_interval_dt, ev_sink);
      } /*make*/
#endif

      /* supplying this to allow for setting up cyclic pointer references */
      EventSink * ev_sink_addr() { return &(this->ev_sink_); }

      /* deliver current event to sink */
      void sink_one() const {
	/* calling .ev_sink() can modify the callback set reentrantly
	 * (i.e. adding/removing callbacks)
	 * although this changes the state of .ev_sink,
	 * we want to treat this as not changing the state of *this
	 */
	RealizationSourceBase * self = const_cast<RealizationSourceBase *>(this);

	self->ev_sink_(this->tracer_->current_ev());
      } /*sink_one*/

      // ----- inherited from ReactorSource -----

      /* process realizations are always primed (at least for now) */
      virtual bool is_empty() const override { return false; }
      /* stochastic process api doesn't have an end time;
       * will need simulator to impose one
       */
      virtual bool is_exhausted() const override { return false; }

      virtual utc_nanos sim_current_tm() const override { return this->tracer_->current_tm(); } 

      virtual std::uint64_t deliver_one() override {
	this->sink_one();
	this->tracer_->advance_dt(this->ev_interval_dt_);

	return 1;
      } /*deliver_one*/

      /* note:
       *   with replay_flag=true,  treats tm as lower bound
       */
      virtual std::uint64_t sim_advance_until(utc_nanos tm, bool replay_flag) override {
	std::uint64_t retval = 0ul;

	if(replay_flag) {
	  while(this->sim_current_tm() < tm) {
	    retval += this->deliver_one();
	  }
	} else {
	  this->tracer_->advance_until(tm);
	}

	return retval;
      } /*advance_until*/
	
      virtual void attach_sink(ref::rp<reactor::AbstractSink> const & /*sink*/) override {
	/* see RealizationSource */
	assert(false);
      }

      virtual void detach_sink(ref::rp<reactor::AbstractSink> const & /*sink*/) override {
	/* see RealizationSource */
	assert(false);
      }

    protected:
      RealizationSourceBase(ref::rp<RealizationTracer<T>> const & tracer,
			    nanos ev_interval_dt,
			    EventSink const & ev_sink)
	: tracer_{tracer},
	  ev_sink_{std::move(ev_sink)},
	  ev_interval_dt_{ev_interval_dt} {}
      RealizationSourceBase(ref::rp<RealizationTracer<T>> const & tracer,
			    nanos ev_interval_dt,
			    EventSink && ev_sink)
	: tracer_{tracer},
	  ev_sink_{std::move(ev_sink)},
	  ev_interval_dt_(ev_interval_dt) {}

    private:
      static constexpr std::string_view sc_self_type = xo::reflect::type_name<RealizationSourceBase<T, EventSink>>();

    private:
      /* produces events representing realized stochastic-process values */
      ref::rp<RealizationTracer<T>> tracer_;
      /* send stochastic-process events to this sink */
      EventSink ev_sink_;
      /* discretize process using this interval:
       * consecutive events from this simulation source will be at least
       * .ev_interval_dt apart
       */
      nanos ev_interval_dt_;
    }; /*RealizationSourceBase*/

    template<typename T>
    class RealizationSource
      : public RealizationSourceBase<T,
				     xo::fn::NotifyCallbackSet<reactor::Sink1<std::pair<xo::time::utc_nanos,T>>,
								  decltype(&reactor::Sink1<std::pair<xo::time::utc_nanos,T>>::notify_ev)>>

    {
    public:
      using utc_nanos = xo::time::utc_nanos;
      using nanos = xo::time::nanos;

      static ref::rp<RealizationSource<T>> make(ref::rp<RealizationTracer<T>> const & tracer,
						nanos ev_interval_dt)
      {
	return new RealizationSource<T>(tracer, ev_interval_dt);
      } /*make*/

      void add_callback(ref::rp<reactor::Sink1<std::pair<utc_nanos,T>>> const & cb) {
	this->ev_sink_addr()->add_callback(cb);
      } /*add_callback*/

      void remove_callback(ref::rp<reactor::Sink1<std::pair<utc_nanos,T>>> const & cb) {
	this->ev_sink_addr()->remove_callback(cb);
      } /*remove_callback*/

      // ----- inherited from AbstractSink -----

      /* alternative naming:
       *    .add_callback(sink)    <--> .attach_sink(sink)
       *    .remove_callback(sink) <--> .detach_sink(sink)
       */
      virtual void attach_sink(ref::rp<reactor::AbstractSink> const & sink) {
	using logutil::xtag;

	/* checking that sink handles events of type T
	 * This is quick-n-dirty.   Want reflection here,   so we can write
	 * a runtime type test
	 *    sink->can_consume<T>()
	 * w/out exploding vtable size
	 */
	constexpr std::string_view c_self_name
	  = "RealizationSource::attach_sink";

	this->add_callback(reactor::Sink1<std::pair<utc_nanos,T>>::require_native
			   (c_self_name, sink));
      } /*attach_sink*/

      virtual void detach_sink(ref::rp<reactor::AbstractSink> const & sink) {
	/* see comment on .attach_sink() */

	constexpr std::string_view c_self_name
	  = "RealizationSource::detach_sink";
	   
	this->remove_callback(reactor::Sink1<std::pair<utc_nanos,T>>::require_native
				(c_self_name, sink));
      } /*detach_sink*/

    private:
      RealizationSource(ref::rp<RealizationTracer<T>> const & tracer,
			nanos ev_interval_dt)
	: RealizationSourceBase
	  <T,
	   xo::fn::NotifyCallbackSet<reactor::Sink1<std::pair<xo::time::utc_nanos,T>>,
				     decltype(&reactor::Sink1<std::pair<xo::time::utc_nanos,T>>::notify_ev)>
	   >(tracer,
	     ev_interval_dt,
	     fn::make_notify_cbset(&reactor::Sink1<std::pair<xo::time::utc_nanos,T>>::notify_ev))
      {}
    }; /*RealizationSource*/

  } /*namespace process*/
} /*namespace xo*/

/* end RealizationSource.hpp */
