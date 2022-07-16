/* @file ExpProcess.hpp */

#pragma once

#include "time/Time.hpp"
#include "process/StochasticProcess.hpp"
#include <memory>
#include <cmath>

namespace xo {
  namespace process {
    // a stochastic process
    //
    //             S(t)
    //   P(t) = m.e
    //
    // where
    // - m    is a constant scale factor
    // - S(t) is some already-defined-and-represented process
    //
    // In particular,  if S(t) is brownian motion,
    // then P(t) is log-normal
    //
    class ExpProcess : public StochasticProcess<double> {
    public:
      static ref::rp<ExpProcess> make(double scale,
				      ref::brw<StochasticProcess<double>> exp_proc) {
	return new ExpProcess(scale, exp_proc);
      } /*make*/

      ref::brw<StochasticProcess<double>> exponent_process() const { return exponent_process_.borrow(); }

      // ----- inherited from StochasticProcess<...> -----

      virtual ~ExpProcess() = default;

      virtual utc_nanos t0() const override { return this->exponent_process_->t0(); }

      virtual double t0_value() const override {
	return this->scale_ * ::exp(this->exponent_process_->t0_value());
      }

      /* note: lo is a sample from the exponentiated process;
       *       must take log to get sample from the exponent process
       */
      virtual value_type exterior_sample(utc_nanos t,
                                         event_type const & lo) override;

      /* note: lo, hi are samples from the exponentiated process;
       *       must take logs to get samples from the exponent process
       */
      virtual value_type interior_sample(utc_nanos t,
					 event_type const & lo,
					 event_type const & hi) override {
	double m
	  = this->scale_;
        double e
	  = (this->exponent_process_->interior_sample
	     (t,
	      event_type(lo.first, ::log(lo.second)),
	      event_type(hi.first, ::log(hi.second))));

        return m * ::exp(e);
      } /*interior_sample*/

      virtual std::string display_string() const override {
	// return tostr("<ExpProcess ",	exponent_process_->display_string(), ">");

	return "<ExpProcess>";
      } /*display_string*/

    private:
      ExpProcess(double scale, ref::brw<StochasticProcess> exp_proc)
	: scale_(scale),
	  exponent_process_{exp_proc.get()} {}
      
    private:
      /* modeling
       *   P(t) = m.exp(E(t))
       * where:
       * - m    is .scale
       * - E(t) is .exponent_process
       */
      double scale_ = 1.0;
      /* exponentiate this process */
      ref::rp<StochasticProcess<double>> exponent_process_;
    }; /*ExpProcess*/
  } /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.hpp */
