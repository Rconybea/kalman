/* @file KalmanFilter.hpp */

#pragma once

#include "filter/KalmanFilterState.hpp"
#include "filter/KalmanFilterTransition.hpp"

namespace xo {
  namespace kalman {
    /* Specification for an ordinary discrete linear kalman filter.
     *
     * The filter generates estimates for a process observed at a discrete
     * set of times tk in {t0, t1, .., tn}
     *
     * At each time tk we have the following:
     *
     * 0. x(0) initial estimate at t(0)
     *    P(0) initial priors: error covariance matrix for x(0)
     *
     * 1. x_(k), [n x 1] vector:
     *    system state, denoted by vector.
     *    (state is not directly observable,
     *     filter will attempt to estimate it)
     *
     * 2. w_(k), [n x 1] vector
     *    Q(k),  [n x n] matrix
     *
     *    w_(k) denotes system noise,
     *    gaussian with covariance Q(k).
     *    noise w_(k) is not directly observable.
     *
     * 3. z(k), [m x 1] vector:
     *
     *    observation vector for time tk
     *
     * 4. v_(k), [m x 1] vector
     *    R(k), [m x m] matrix
     *
     *    v_(k) denotes observation errors,
     *    gaussian with covariance R(k).
     *    noise v_(k) is not directly observable.
     *
     * 5. F(k), [n x n] matrix
     *    state transition matrix
     *    model system evolves according to:
     *
     *    x_(k+1) = F(x).x_(k) + w_(k)
     *
     * 6. observations z(k) depend on system state:
     *
     *    z(k) = H(k).x_(k) + v_(k)    
     *     
     * 7. Kalman filter outputs:
     *    x(k), [n x 1] vector
     *    Q(k), [n x n] matrix
     *
     *    x(k) is optimal estimate for system state x_(k)
     *    P(k) is covariance matrix specifying confidence intervals
     *         for pairs (x(k)[i], x(k)[j])
     *
     * filter specification consists of:
     *    n, m, x(0), P(0), F(k), Q(k), H(k), R(k)
     * The cardinality of observations z(k) can vary over time,
     * so to be precise,  m can vary with tk;   write as m(k)
     *
     * More details:
     * - avoid having to specify t(k) in advance;
     *   instead defer until observation available
     *   so t(k) can be taken from polling timestamp
     */

    class KalmanFilterObservable {
    public:
      using MatrixXd = Eigen::MatrixXd;
      
    public:
      KalmanFilterObservable() = default;
      KalmanFilterObservable(MatrixXd H, MatrixXd R)
	: H_{std::move(H)}, R_{std::move(R)} {
	assert(this->check_ok());
      } /*ctor*/

      uint32_t n_state() const { return H_.cols(); }
      uint32_t n_observable() const { return H_.rows(); }
      MatrixXd const & observable() const { return H_; }
      MatrixXd const & observable_cov() const { return R_; }
      
      bool check_ok() const {
	uint32_t m = H_.rows();
	bool r_is_mxm = ((R_.cols() == m) && (R_.rows() == m));

	/* also would like to require: R is +ve definite */
	
	return r_is_mxm;
      } /*check_ok*/

    private:
      /* [m x n] observation matrix */
      MatrixXd H_;
      /* [m x m] covariance matrix for observation noise */
      MatrixXd R_;
    }; /*KalmanFilterObservable*/

    class KalmanFilterInput {
    public:
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;
      using uint32_t = std::uint32_t;

    public:
      KalmanFilterInput() = default;
      explicit KalmanFilterInput(utc_nanos tkp1, VectorXd z)
	: tkp1_(tkp1), z_{std::move(z)} {}

      utc_nanos tkp1() const { return tkp1_; }
      uint32_t n_obs() const { return z_.size(); }
      VectorXd const & z() const { return z_; }

    private:
      /* t(k+1) - asof time for observations .z */
      utc_nanos tkp1_ = xo::time::Time::epoch();
      /* [m x 1] observation vector z(k) */
      VectorXd z_;
    }; /*KalmanFilterInput*/

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

    private:
      /* system state: timestamp, estimated process state, process covariance
       *               asof beginning of this step
       */
      KalmanFilterState state_;
      /* input: observations at time t(k+1) */
      KalmanFilterInput input_;
    }; /*KalmanFilterStep*/

    /* full specification for a kalman filter.
     *
     * For a textbook linear filter,  expect a KalmanFilterStep
     * instance to be independent of KalmanFilterState/KalmanFilterInput.
     * 
     * Relaxing this requirement for two reasons:
     * 1. (proper) want to allow filter with variable timing between observations,
     *    expecially if observations are event-driven.
     *    In that case it's likely that state transition matrices are a function
     *    of elapsed time between observations.  Providing filter state sk
     *    allows MkStepFn to use sk.tm()
     * 2. (sketchy) when observations represent market data,
     *    desirable to treat an observation as giving one-sided information
     *    about true value.   For example treat a bid price as evidence 
     *    true value is higher than that bid,  but don't want to constrain
     *    "how much higher".   Certainly no reason to think that
     *    bid price is normally distributed around fair value.
     *    Allow for hack in which we 
     *    and modulate error distribution "as if it were normal" to assess
     *    a non-gaussian error distribution
     */
    class KalmanFilterSpec {
    public:
      using MkStepFn = std::function<KalmanFilterStep
				     (KalmanFilterState const & sk,
	                              KalmanFilterInput const & zkp1)>;

    public:
      explicit KalmanFilterSpec(KalmanFilterStateExt s0, MkStepFn mkstepfn)
	: start_ext_{std::move(s0)}, mk_step_fn_{std::move(mkstepfn)} {}

      KalmanFilterStateExt const & start_ext() const { return start_ext_; }
      /* get step parameters (i.e. matrices F, Q, H, R)
       * for step t(k) -> t(k+1).
       *
       * We supply t(k) state s and t(k+1) observation z(k+1)
       * to allow time stepping to be observation-driven
       */
      KalmanFilterStep make_step(KalmanFilterState const & sk,
				 KalmanFilterInput const & zkp1) {
	return this->mk_step_fn_(sk, zkp1);
      } /*make_step*/

    public:
      /* starting state */
      KalmanFilterStateExt start_ext_;

      /* creates kalman filter step object on demand;
       * step object specifies inputs to 1 step in discrete
       * linear kalman filter
       */
      MkStepFn mk_step_fn_;
    }; /*KalmanFilterSpec*/

    class KalmanFilterEngine {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* evolution of system state + account for system noise,
       * before incorporating effect of observations z(k+1)
       *   x(k) --> x(k+1|k)
       *   P(k) --> P(k+1|k)
       *
       * tkp1.  time t(k+1) assoc'd with step k+1
       * sk.    filter state at time tk:
       *    sk.k  = k       step # (starts from 0)
       *    sk.tk = t(k)    time t(k) assoc'd with step #k
       *    sk.x = x(k)     estimated state at time tk
       *    sk.P = P(k)     quality of state estimate x(k)
       *                    (error covariance matrix)
       * Fk. state transition:
       *    Fk.F = F(k)     state transition matrix
       *    Fk.Q = Q(k)     covariance matrix for system noise
       *
       * returns propagated state estimate for t(k+1):
       *    retval.k = k+1
       *    retval.tk = t(k+1) = tkp1
       *    retval.x = x(k+1|k)
       *    retval.P = P(k+1|k)
       */
      static KalmanFilterState extrapolate(utc_nanos tkp1,
					   KalmanFilterState const & sk,
					   KalmanFilterTransition const & Fk);

      /* compute kalman gain matrix for filter step t(k) -> t(k+1)
       * Expensive implementation using matrix inversion
       *
       *                              T
       *   M(k+1) = H(k).P(k+1|k).H(k) + R(k)
       *
       *                         T       -1
       *   K(k+1) = P(k+1|k).H(k) .M(k+1)
       *
       * Require:
       * - skp1_ext.n_state() = Hkp1.n_state()
       *
       * skp1_ext.  extrapolated filter state at t(k+1)
       * Hkp1.      relates model state to observable variables,
       *            for step t(k) -> t(k+1)
       */
      static MatrixXd kalman_gain(KalmanFilterState const & skp1_ext,
				  KalmanFilterObservable const & Hkp1);

      /* compute kalman gain for a single observation z(k)[j].
       * This is useful iff the observation error matrix R is diagonal.
       * For diagonal R we can present a set of observations z(k) serially
       * instead of all at once,  with lower time complexity
       *
       * Kalman Filter specifies some space with m observables.
       * j identifies one of those observables, indexing from 0.
       * This corresponds to row #j of H(k), and element R[j,j] of R.
       *
       * Effectively,  we are projecting the kalman filter assoc'd with
       * {skp1_ext, Hkp1} to a filter with a single observable variable z(k)[j],
       * then computing the (scalar) kalman gain for this 1-variable filter
       *
       * The gain vector tells us for each member of filter state,
       * how much to adjust our optimal estimate for that member for a unit
       * amount of innovation in observable #j,  i.e. for difference between
       * expected and actual value for that observable.
       */
      static VectorXd kalman_gain1(KalmanFilterState const & skp1_ext,
				   KalmanFilterObservable const & Hkp1,
				   uint32_t j);

      /* correct extrapolated filter state for observation
       * of j'th filter observable z(k+1)[j]
       *
       * Can use this when observation errors are uncorrelated
       * (i.e. observation error matrix R is diagonal)
       */
      static KalmanFilterStateExt correct1(KalmanFilterState const & skp1_ext,
					   KalmanFilterObservable const & Hkp1,
					   KalmanFilterInput const & zkp1,
					   uint32_t j);

      /* correct extrapolated state+cov estimate;
       * also computes kalman gain
       *
       * Require:
       * - skp1_ext.n_state() = Hkp1.n_state()
       * - zkp1.n_obs() == Hkp1.n_observable()
       *
       * skp1_ext.      extrapolated kalman state + covaraince at t(k+1)
       * Hkp1.          relates model state to observable variables
       *                for step t(k) -> t(k+1)
       * zkp1.          observations for time t(k+1)
       *
       * return new filter state+cov
       */
      static KalmanFilterStateExt correct(KalmanFilterState const & skp1_ext,
					  KalmanFilterObservable const & Hkp1,
					  KalmanFilterInput const & zkp1);

      /* step filter from t(k) -> t(k+1)
       *
       * sk.        filter state from previous step:
       *            x (state vector), P (state covar matrix)
       * Fk.        transition-related params:
       *            F (transition matrix), Q (system noise covar matrix)
       * Hkp1.      observation-related params:
       *            H (coupling matrix), R (error covar matrix)
       * zkp1.      observations z(k+1) for time t(k+1)
       */
      static KalmanFilterStateExt step(utc_nanos tkp1,
                                       KalmanFilterState const & sk,
                                       KalmanFilterTransition const & Fk,
                                       KalmanFilterObservable const & Hkp1,
                                       KalmanFilterInput const & zkp1);

      /* step filter from t(k) -> tk(k+1)
       * same as
       *   .step(tkp1, sk, step_spec.model(), step_spec.obs(), zkp1);
       *
       * step_spec.  encapsulates Fk (transition-related params)
       *             and Q (system noise covar matrix)
       */
      static KalmanFilterStateExt step(KalmanFilterStep const & step_spec);

      /* step filter from t(k) -> t(k+1)
       *
       * sk.    filter state from previous step:
       *        x (state vector), P (state covar matrix)
       * Fk.    transition-related params:
       *        F (transition matrix), Q (system noise covar matrix)
       * Hkp1.  observation-related params:
       *        H (coupling matrix), R (error covar matrix)
       * zkp1.  observations z(k+1) for time t(k+1)
       * j.     identifies a single filter observable --
       *        step will only consume observation z(k+1)[j]
       */
      static KalmanFilterStateExt step1(utc_nanos tkp1,
					KalmanFilterState const & sk,
					KalmanFilterTransition const & Fk,
					KalmanFilterObservable const & Hkp1,
					KalmanFilterInput const & zkp1,
					uint32_t j);

      /* step filter from t(k) -> t(k+1)
       *
       * same as
       *   .step1(step_spec.tkp1(),
       *          step_spec.state(),
       *          step_spec.model(),
       *          step_spec.obs(),
       *          step_spec.input(),
       *          j);
       *
       * step_spec.  encapsulates
       *             x (state vector),
       *             P (state covar matrix),
       *             Fk (transition-related params),
       *             Q (system noise covar matrix)
       *             z (z(k+1), observations at time t(k+1))
       * j.          identifies a single filter observable --
       *             step will only consume observation z(k+1)[j]
       */
      static KalmanFilterStateExt step1(KalmanFilterStep const & step_spec,
					uint32_t j);
    }; /*KalmanFilterEngine*/

    /* encapsulate a (linear) kalman filter
     * together with event publishing
     */
    class KalmanFilter {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* create filter with specification given by spec,  and initial state s0 */
      explicit KalmanFilter(KalmanFilterSpec spec);

      uint32_t step_no() const { return state_ext_.step_no(); }
      utc_nanos tm() const { return state_ext_.tm(); }
      KalmanFilterSpec const & filter_spec() const { return filter_spec_; }
      KalmanFilterStep const & step() const { return step_; }
      KalmanFilterStateExt const & state_ext() const { return state_ext_; }

      /* notify kalman filter with input for time t(k+1) = input_kp1.tkp1()
       * Require: input.tkp1() >= .current_tm()
       * Promise:
       * - .tm() = input_kp1.tkp1()
       * - .step_no() = old .step_no() + 1
       * - .filter_spec_k, .step_k, .state_k updated
       *   for observations in input_kp1
       */
      void notify_input(KalmanFilterInput const & input_kp1);

    private:
      /* specification for kalman filter;
       * produces process/observation matrices on demand
       */
      KalmanFilterSpec filter_spec_;

      /* filter step for most recent observation */
      KalmanFilterStep step_;

      /* filter state as of most recent observation;
       * result of applying KalmanFilterEngine::step() to contents of .step
       */
      KalmanFilterStateExt state_ext_;
    }; /*KalmanFilter*/
  } /*namespace kalman*/
} /*namespace xo*/


/* end KalmanFilter.hpp */
