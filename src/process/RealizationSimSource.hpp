/* @file RealizationSimSource.hpp */

#pragma once

#include "simulator/SimulationSource.hpp"
#include "process/RealizationTracer.hpp"
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
    class RealizationSimSource : public xo::reactor::Source {
    public:
      using nanos = xo::time::nanos;

    public:
      ~RealizationSimSource() {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "RealizationSimSource<>::dtor";
	constexpr bool c_logging_enabled = false;
	
	scope lscope(c_self, c_logging_enabled);
	if(c_logging_enabled)
	  lscope.log("delete instance", xtag("p", this));
      } /*dtor*/

      static ref::rp<RealizationSimSource> make(ref::rp<RealizationTracer<T>> const & tracer,
						nanos ev_interval_dt,
						EventSink const & ev_sink)
      {
	using logutil::scope;
	using logutil::xtag;

	constexpr bool c_logging_enabled = true;

	auto p = new RealizationSimSource(tracer, ev_interval_dt, ev_sink);

	scope lscope(sc_self_type, "::make", c_logging_enabled);
	if(c_logging_enabled)
	  lscope.log("create instance",
		     xtag("p", p),
		     xtag("bytes", sizeof(RealizationSimSource)));

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
	std::invoke(this->ev_sink_, this->tracer_->current_ev());
      } /*sink_one*/

      // ----- inherited from Source -----

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
	
    private:
      RealizationSimSource(ref::rp<RealizationTracer<T>> const & tracer,
			   nanos ev_interval_dt,
			   EventSink const & ev_sink)
	: tracer_{tracer},
	  ev_sink_{std::move(ev_sink)},
	  ev_interval_dt_{ev_interval_dt} {}
      RealizationSimSource(ref::rp<RealizationTracer<T>> const & tracer,
			   nanos ev_interval_dt,
			   EventSink && ev_sink)
	: tracer_{tracer},
	  ev_sink_{std::move(ev_sink)},
	  ev_interval_dt_(ev_interval_dt) {}

    private:
      static constexpr std::string_view sc_self_type = xo::reflect::type_name<RealizationSimSource<T, EventSink>>();

    private:
      /* produces events representing realized stochastic-process values */
      ref::rp<RealizationTracer<T>> tracer_;
      /* consume events coming from this sim source */
      EventSink ev_sink_;
      /* discretize process using this interval:
       * consecutive events from this simulation source will be at least
       * .ev_interval_dt apart
       */
      nanos ev_interval_dt_;
    }; /*RealizationSimSource*/
  } /*namespace process*/
} /*namespace xo*/

/* end RealizationSimSource.hpp */
