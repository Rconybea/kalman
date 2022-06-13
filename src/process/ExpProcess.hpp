/* @file ExpProcess.hpp */

#pragma once

#include "process/StochasticProcess.hpp"
#include <memory>
#include <cmath>

namespace xo {
  namespace process {
    // a stochastic process
    //
    //           S(t)
    //   P(t) = e
    //
    // where S(t) is some already-defined-and-represented process
    //
    // In particular,  if S(t) is brownian motion,
    // then P(t) is log-normal
    //
    class ExpProcess : public StochasticProcess<double> {
    public:
      ExpProcess(std::unique_ptr<StochasticProcess> exp_process)
	: exponent_process_{std::move(exp_process)} {}
      
      static std::unique_ptr<ExpProcess> make(std::unique_ptr<StochasticProcess<double>> exp_process) {
	return std::unique_ptr<ExpProcess>(new ExpProcess(std::move(exp_process)));
      } /*make*/

      StochasticProcess<double> * exponent_process() const { return exponent_process_.get(); }

      // ----- inherited from StochasticProcess<...> -----

      virtual ~ExpProcess() = default;

      virtual utc_nanos t0() const override { return this->exponent_process_->t0(); }

      virtual double t0_value() const override {
	return ::exp(this->exponent_process_->t0_value());
      }

      /* note: lo is a sample from the exponentiated process;
       *       must take log to get sample from the exponent process
       */
      virtual value_type exterior_sample(utc_nanos t,
                                         event_type const &lo) override {
        double e
	  = (this->exponent_process_->exterior_sample
	     (t,
	      event_type(lo.first, ::log(lo.second))));

        return ::exp(e);
      } /*exterior_sample*/

      /* note: lo, hi are samples from the exponentiated process;
       *       must take logs to get samples from the exponent process
       */
      virtual value_type interior_sample(utc_nanos t,
					 event_type const & lo,
					 event_type const & hi) override {
        double e
	  = (this->exponent_process_->interior_sample
	     (t,
	      event_type(lo.first, ::log(lo.second)),
	      event_type(hi.first, ::log(hi.second))));

        return ::exp(e);
      } /*interior_sample*/

    private:
      std::unique_ptr<StochasticProcess<double>> exponent_process_ = nullptr;
    }; /*ExpProcess*/
  } /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.hpp */
