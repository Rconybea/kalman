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
     * - EventSink(T)
     */
    template <typename T, typename EventSink>
    class RealizationSimSource : public xo::sim::SimulationSource {
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

      static refcnt::rp<RealizationSimSource> make(RealizationTracer<T> * tracer,
						   nanos ev_interval_dt,
						   EventSink const & ev_sink)
      {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "RealizationSimSource<>::make";
	constexpr bool c_logging_enabled = false;

	auto p = new RealizationSimSource(tracer, ev_interval_dt, ev_sink);

	scope lscope(c_self, c_logging_enabled);
	if(c_logging_enabled)
	  lscope.log("create instance",
		     xtag("p", p),
		     xtag("bytes", sizeof(RealizationSimSource)));

	return p;
      } /*make*/

      static refcnt::rp<RealizationSimSource> make(RealizationTracer<T> * tracer,
						   nanos ev_interval_dt,
						   EventSink && ev_sink)
      {
	return new RealizationSimSource(tracer, ev_interval_dt, ev_sink);
      } /*make*/

      /* deliver current event to sink */
      void sink_one() const {
	std::invoke(this->ev_sink_, this->tracer_->current_ev());
      } /*sink_one*/

      // ----- inherited from SimulationSource -----

      /* stochastic process api doesn't have an end time;
       * will need simulator to impose one
       */
      virtual bool is_exhausted() const override { return false; }
      virtual utc_nanos current_tm() const override { return this->tracer_->current_tm(); } 

      /* note:
       *   with replay_flag=true,  treats tm as lower bound
       */
      virtual void advance_until(utc_nanos tm, bool replay_flag) override {
	if(replay_flag) {
	  while(this->current_tm() < tm) {
	    this->advance_one();
	  }
	} else {
	  this->tracer_->advance_until(tm);
	}
      } /*advance_until*/
	
      virtual std::uint64_t advance_one() override {
	this->sink_one();
	this->tracer_->advance_dt(this->ev_interval_dt_);

	return 1;
      } /*advance_one*/

    private:
      RealizationSimSource(RealizationTracer<T> * tracer,
			   nanos ev_interval_dt,
			   EventSink const & ev_sink)
	: tracer_(tracer),
	  ev_sink_(ev_sink),
	  ev_interval_dt_(ev_interval_dt) {}
      RealizationSimSource(RealizationTracer<T> * tracer,
			   nanos ev_interval_dt,
			   EventSink && ev_sink)
	: tracer_(tracer),
	  ev_sink_{std::move(ev_sink)},
	  ev_interval_dt_(ev_interval_dt) {}

    private:
      /* produces events representing realized stochastic-process values */
      RealizationTracer<T> * tracer_ = nullptr;
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
