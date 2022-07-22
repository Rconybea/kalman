/* @file KalmanFilterStep.hpp */

#pragma once

#include "KalmanFilterState.hpp"
#include "KalmanFilterInput.hpp"
#include "KalmanFilterTransition.hpp"
#include "KalmanFilterObservable.hpp"

namespace xo {
  namespace kalman {
    /* encapsulate {state + observation} models for a single time step t(k).
     * Emitted by KalmanFilterSpec, q.v.
     */
    class KalmanFilterStepBase {
    public:
      KalmanFilterStepBase() = default;
      KalmanFilterStepBase(KalmanFilterTransition model,
			   KalmanFilterObservable obs)
	: model_{std::move(model)},
	  obs_{std::move(obs)} {}

      KalmanFilterTransition const & model() const { return model_; }
      KalmanFilterObservable const & obs() const { return obs_; }

    private:
      /* model for process being observed (state transition + noise) */
      KalmanFilterTransition model_;
      /* what can be observed (observables + noise) */
      KalmanFilterObservable obs_;
    }; /*KalmanFilterStepBase*/

    /* encapsulate {state + observation} models for a single time step t(k).
     * Emitted by KalmanFilterSpec, q.v.
     */
    class KalmanFilterStep : public KalmanFilterStepBase {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      KalmanFilterStep() = default;
      KalmanFilterStep(KalmanFilterState state,
		       KalmanFilterTransition model,
		       KalmanFilterObservable obs,
		       KalmanFilterInput zkp1)
	: KalmanFilterStepBase(model, obs),
	  state_{std::move(state)},
	  input_{std::move(zkp1)} {}
      
      KalmanFilterState const & state() const { return state_; }
      KalmanFilterInput const & input() const { return input_; }

      utc_nanos tkp1() const { return input_.tkp1(); }

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      /* system state: timestamp, estimated process state, process covariance
       *               asof beginning of this step
       */
      KalmanFilterState state_;
      /* input: observations at time t(k+1) */
      KalmanFilterInput input_;
    }; /*KalmanFilterStep*/

    inline std::ostream &
    operator<<(std::ostream & os, KalmanFilterStep const & x) {
      x.display(os);
      return os;
    } /*operator<<*/

  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStep.hpp */
