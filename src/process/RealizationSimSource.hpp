/* @file RealizationSimSource.hpp */

#pragma once

#include "simulator/SimulationSource.hpp"
#include "process/RealizationTracer.hpp"
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
      RealizationSimSource(RealizationTracer<T> * tracer, EventSink const & ev_sink)
	: tracer_(tracer), ev_sink_(ev_sink) {}
      RealizationSimSource(RealizationTracer<T> * tracer, EventSink && ev_sink)
	: tracer_(tracer), ev_sink_{std::move(ev_sink)} {}

      /* deliver current event to sink */
      void deliver_one() const {
	std::invoke(this->ev_sink_, this->tracer_->current_ev());
      } /*deliver_one*/

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
	
      virtual void advance_one() override {
	this->deliver_one();
	this->tracer_->advance_dt(this->ev_interval_dt_);
      }

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
