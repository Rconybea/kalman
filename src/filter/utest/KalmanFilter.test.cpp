/* @file KalmanFilter.test.cpp */

#include "filter/KalmanFilter.hpp"
#include "statistics/SampleStatistics.hpp"
#include "random/Normal.hpp"
#include "random/xoshiro256.hpp"
#include "logutil/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
  using xo::kalman::KalmanFilterSpec;
  using xo::kalman::KalmanFilterStep;
  using xo::kalman::KalmanFilterEngine;
  using xo::kalman::KalmanFilterStateExt;
  using xo::kalman::KalmanFilterState;
  using xo::kalman::KalmanFilterTransition;
  using xo::kalman::KalmanFilterObservable;
  using xo::kalman::KalmanFilterInput;
  using xo::statistics::SampleStatistics;
  using xo::random::NormalGen;
  using xo::random::xoshiro256ss;
  using xo::time::Time;
  using xo::time::utc_nanos;
  using xo::time::seconds;
  using logutil::tostr;
  using Eigen::MatrixXd;
  using Eigen::VectorXd;

  namespace ut {
    namespace {
      KalmanFilterSpec::MkStepFn
      kalman_identity_mkstep_fn()
      {
	/* kalman state transition matrix: use identity <--> state is constant */
	MatrixXd F = MatrixXd::Identity(1, 1);

	/* state transition noise: set this to zero;
	 * measuring something that's known to be constant
	 */
	MatrixXd Q = MatrixXd::Zero(1, 1);

	/* single direct observation */
	MatrixXd H = MatrixXd::Identity(1, 1);

	/* observation errors understood to have
	 * mean 0, sdev 1
	 *
	 * This is consistent with normal_rng below,
	 * so R is correctly specified
	 */
	MatrixXd R = MatrixXd::Identity(1, 1);

	return [F, Q, H, R](KalmanFilterState const & sk,
			    KalmanFilterInput const & zkp1) {
	  KalmanFilterTransition Fk(F, Q);
	  KalmanFilterObservable Hk(H, R);

	  return KalmanFilterStep(sk, Fk, Hk, zkp1);
	};
      } /*kalman_identity_mkstep_fn*/
    } /*namespace*/

    /* example 1.
     *   repeated direct observation of a scalar
     *   use rng to generate observations
     */
    TEST_CASE("kalman-identity", "[kalmanfilter]") {
      /* setting up trivial filter for repeated indept
       * measurements of a constant.
       *
       * True value of unknown set to 10,
       * utest observes filter converging toward that value
       */

      /* seed for rng */
      uint64_t seed = 14950319842636922572UL;

      /* N(0,1) random numbers */
      auto normal_rng = NormalGen<xoshiro256ss>::make(seed,
						      0.0 /*mean*/,
						      1.0 /*sdev*/);

      /* accumulate statistics on 'measurements',
       * use as reference implementation for filter
       */
      SampleStatistics z_stats;

      utc_nanos t0 = Time::ymd_midnight(20220707);

      /* estimate x(0) = [0] */
      VectorXd x0(1);
      x0 << 10.0 + normal_rng();

      INFO(tostr("x0=", x0));

      z_stats.include_sample(x0[0]);

      /* kalman prior : Variance = 1, sdev = 1 */
      MatrixXd P0 = 1.0 * MatrixXd::Identity(1, 1);

      KalmanFilterStateExt s0(0 /*step#*/,
			      t0,
			      x0,
			      P0,
			      MatrixXd::Zero(1, 1) /*K*/,
			      -1 /*j*/);

      auto mk_step_fn
	= kalman_identity_mkstep_fn();

      KalmanFilterSpec spec(s0, mk_step_fn);

      KalmanFilterStateExt sk = spec.start_ext();

      for(uint32_t i_step = 1; i_step < 100; ++i_step) {
	/* note: for this filter,  measurement time doesn't matter */
	utc_nanos tkp1 = sk.tm() + seconds(1);

	VectorXd z(1);
	z << 10.0 + normal_rng();

	INFO(tostr("z=", z));

	z_stats.include_sample(z[0]);

	KalmanFilterInput inputk(tkp1, z);

	KalmanFilterStep step_spec = spec.make_step(sk, inputk);

	KalmanFilterStateExt skp1
	  = KalmanFilterEngine::step(step_spec);

	REQUIRE(skp1.step_no() == i_step);
	REQUIRE(skp1.tm() == tkp1);
	REQUIRE(skp1.n_state() == 1);
	REQUIRE(skp1.state_v().size() == 1);
	REQUIRE(skp1.state_v()[0] == Approx(z_stats.mean()).epsilon(1e-6));
	REQUIRE(skp1.state_cov().rows() == 1);
	REQUIRE(skp1.state_cov().cols() == 1);
	REQUIRE(skp1.gain().rows() == 1);
	REQUIRE(skp1.gain().cols() == 1);
	REQUIRE(skp1.observable() == -1);

	/* z_stats reflects k = z_stats.n_sample() N(0,1) 'random' vars;
	 * variance of sum (i.e. z_stats.mean() * k) is proportional to k;
	 * variance of mean like 1/k
	 *
	 * kalman filter also should compute covariance estimate like 1/k
	 */

	REQUIRE(skp1.state_cov()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));

	REQUIRE(skp1.gain()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));

	/* estimate at each step should be (approximately)
	 * average of measurements taken so far.
	 * approximate because also affected by prior
	 */

	sk = skp1;
      }

      REQUIRE(sk.state_v()[0] == Approx(10.0).epsilon(1e-2));
      REQUIRE(sk.state_cov()(0, 0) == Approx(0.01).epsilon(1e-6));
      REQUIRE(sk.gain()(0, 0) == Approx(0.01).epsilon(1e-6));
    } /*TEST_CASE(kalman-identity)*/

    /* example 2.
     *   like example 1, but using "separate observation" variants:
     *     KalmanGain::correct1()     // per observation
     *   instead of
     *     KalmanGain::correct()      // per observation set
     */
    TEST_CASE("kalman-identity1", "[kalmanfilter]") {
      /* setting up trivial filter for repeated indept
       * measurements of a constant.
       *
       * True value of unknown set to 10,
       * utest observes filter converging toward that value
       *       
       */

      /* seed for rng */
      uint64_t seed = 14950319842636922572UL;

      /* N(0,1) random numbers */
      auto normal_rng = NormalGen<xoshiro256ss>::make(seed,
						      0.0 /*mean*/,
						      1.0 /*sdev*/);

      /* accumulate statistics on 'measurements',
       * use as reference implementation for filter
       */
      SampleStatistics z_stats;

      utc_nanos t0 = Time::ymd_midnight(20220707);

      /* estimate x(0) = [0] */
      VectorXd x0(1);
      x0 << 10.0 + normal_rng();

      INFO(tostr("x0=", x0));

      z_stats.include_sample(x0[0]);

      /* kalman prior : Variance = 1, sdev = 1 */
      MatrixXd P0 = 1.0 * MatrixXd::Identity(1, 1);

#ifdef OBSOLETE
      /* kalman state transition matrix: use identity <--> state is constant */
      MatrixXd F = MatrixXd::Identity(1, 1);

      /* state transition noise: set this to zero;
       * measuring something that's known to be constant
       */
      MatrixXd Q = MatrixXd::Zero(1, 1);

      /* single direct observation */
      MatrixXd H = MatrixXd::Identity(1, 1);

      /* observation errors understood to have
       * mean 0, sdev 1
       *
       * This is consistent with normal_rng below,
       * so R is correctly specified
       */
      MatrixXd R = MatrixXd::Identity(1, 1);
#endif

#ifdef OBSOLETE
      /* for this filter,  transition + observable stages are constant */
      KalmanFilterTransition Fk
	= KalmanFilterTransition(F, Q);
      KalmanFilterObservable Hk
	= KalmanFilterObservable(H, R);
#endif

      KalmanFilterStateExt s0(0 /*step#*/,
			      t0,
			      x0,
			      P0,
			      MatrixXd::Zero(1, 1) /*K*/,
			      -1);

      auto mk_step_fn
	= kalman_identity_mkstep_fn();

      KalmanFilterSpec spec(s0, mk_step_fn);

      KalmanFilterStateExt sk = spec.start_ext();

      for(uint32_t i_step = 1; i_step < 100; ++i_step) {
	/* note: for this filter,  measurement time doesn't matter */
	utc_nanos tkp1 = sk.tm() + seconds(1);

	VectorXd z(1);
	z << 10.0 + normal_rng();

	INFO(tostr("z=", z));

	z_stats.include_sample(z[0]);

	KalmanFilterInput inputk(tkp1, z);

	KalmanFilterStep step_spec
	  = spec.make_step(sk, inputk);

	KalmanFilterStateExt skp1
	  = KalmanFilterEngine::step1(step_spec, 0 /*j*/);

	REQUIRE(skp1.step_no() == i_step);
	REQUIRE(skp1.tm() == tkp1);
	REQUIRE(skp1.n_state() == 1);
	REQUIRE(skp1.state_v().size() == 1);
	REQUIRE(skp1.state_v()[0] == Approx(z_stats.mean()).epsilon(1e-6));
	REQUIRE(skp1.state_cov().rows() == 1);
	REQUIRE(skp1.state_cov().cols() == 1);
	REQUIRE(skp1.gain().rows() == 1);
	REQUIRE(skp1.gain().cols() == 1);
	REQUIRE(skp1.observable() == 0);

	/* z_stats reflects k = z_stats.n_sample() N(0,1) 'random' vars;
	 * variance of sum (i.e. z_stats.mean() * k) is proportional to k;
	 * variance of mean like 1/k
	 *
	 * kalman filter also should compute covariance estimate like 1/k
	 */

	REQUIRE(skp1.state_cov()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));

	REQUIRE(skp1.gain()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));

	/* estimate at each step should be (approximately)
	 * average of measurements taken so far.
	 * approximate because also affected by prior
	 */

	sk = skp1;
      }

      REQUIRE(sk.state_v()[0] == Approx(10.0).epsilon(1e-2));
      REQUIRE(sk.state_cov()(0, 0) == Approx(0.01).epsilon(1e-6));
      REQUIRE(sk.gain()(0, 0) == Approx(0.01).epsilon(1e-6));
    } /*TEST_CASE(kalman-identity1)*/



  } /*namespace ut*/
} /*namespace xo*/

/* end KalmanFilter.test.cpp */

