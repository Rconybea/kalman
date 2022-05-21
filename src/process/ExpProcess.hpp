/* @file ExpProcess.hpp */

#pragma once

#include "process/StochasticProcess.hpp"
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
      ExpProcess(StochasticProcess * exp_process)
	: exponent_process_(exp_process) {}
      
      static ExpProcess * make(StochasticProcess * exp_process) {
	return new ExpProcess(exp_process);
      } /*make*/

      virtual ~ExpProcess() = default;

      virtual utc_nanos t0() const override { return this->exponent_process_->t0(); }

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
					 event_type const & hi) override;

    private:
      StochasticProcess<double> * exponent_process_ = nullptr;
    }; /*ExpProcess*/
  } /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.hpp */
