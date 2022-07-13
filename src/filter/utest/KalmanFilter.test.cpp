/* @file KalmanFilter.test.cpp */

#include "filter/KalmanFilter.hpp"
#include "filter/print_eigen.hpp"
#include "statistics/SampleStatistics.hpp"
#include "random/Normal.hpp"
#include "random/xoshiro256.hpp"
#include "logutil/scope.hpp"
#include <catch2/catch.hpp>
#include <fstream>

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
  using logutil::matrix;
  using logutil::scope;
  using logutil::tostr;
  using logutil::xtag;
  using Eigen::MatrixXd;
  using Eigen::VectorXd;

  namespace ut {
    namespace {
      /* step for kalman filter with:
       * - single state variable x[0]
       * - identity process model - x(k+1) = F(k).x(k), with F(k) = | 1 |
       * - no process noise
       * - single observation z[0]
       * - identity coupling matrix - z(k) = H(k).x(k) + w(k), with H(k) = | 1 |
       */
      KalmanFilterSpec::MkStepFn
      kalman_identity1_mkstep_fn()
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
      } /*kalman_identity1_mkstep_fn*/
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
	= kalman_identity1_mkstep_fn();

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
	= kalman_identity1_mkstep_fn();

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

    namespace {
      /* step for kalman filter with:
       * - single state variable x[0]
       * - identity process model: x(k+1) = F(k).x(k), with F(k) = | 1 |
       * - no process noise
       * - two observations z[0], z[1]
       * - identity coupling matrix: z(k) = H(k).x(k) + w(k), with
       *     H(k) = | 1 |
       *            | 1 |
       *
       *     w(k) = | w1 |  with w1 ~ N(0,1)
       *            | w2 |
       */
      KalmanFilterSpec::MkStepFn
      kalman_identity2_mkstep_fn()
      {
	/* kalman state transition matrix: use identity <-> state is constant */
	MatrixXd F = MatrixXd::Identity(1, 1);

	/* state transition noise: set to 0 */
	MatrixXd Q = MatrixXd::Zero(1, 1);

	/* two direct observations */
	MatrixXd H = MatrixXd::Constant(2 /*#rows*/, 1 /*#cols*/, 1.0 /*M(i,j)*/);
      
	/* observation errors: N(0,1) */
	MatrixXd R = MatrixXd::Identity(2, 2);

	return [F, Q, H, R](KalmanFilterState const & sk,
			    KalmanFilterInput const & zkp1) {
	  KalmanFilterTransition Fk(F, Q);
	  KalmanFilterObservable Hk(H, R);

	  return KalmanFilterStep(sk, Fk, Hk, zkp1);
	};
      } /*kalman_identity2_mkstep_fn*/
    } /*namespace*/

    TEST_CASE("kalman-identity2", "[kalmanfilter]") {
      /* variation on filter in kalman-identity1 utest above;
       * this time make 2 observations per step 
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
	= kalman_identity2_mkstep_fn();

      KalmanFilterSpec spec(s0, mk_step_fn);
      KalmanFilterStateExt sk = spec.start_ext();

      /* need 1/2 as many filter steps to reach same confidence
       * as in test "kalman-identity"
       */
      for(uint32_t i_step = 1; i_step < 51; ++i_step) {
	INFO(tostr(xtag("i_step", i_step)));

	/* note: for this filter, measurement time doesn't affect behavior */
	utc_nanos tkp1 = sk.tm() + seconds(1);

	VectorXd z(2);
	z << 10.0 + normal_rng(), 10.0 + normal_rng();

	z_stats.include_sample(z[0]);
	z_stats.include_sample(z[1]);
      
	INFO(tostr(xtag("i_step", i_step), xtag("z", z)));

	KalmanFilterInput inputk(tkp1, z);

	KalmanFilterStep step_spec = spec.make_step(sk, inputk);

	KalmanFilterStateExt skp1 = KalmanFilterEngine::step(step_spec);

	REQUIRE(skp1.step_no() == i_step);
	REQUIRE(skp1.tm() == tkp1);
	REQUIRE(skp1.n_state() == 1);
	REQUIRE(skp1.state_v().size() == 1);
	REQUIRE(skp1.state_v()[0] == Approx(z_stats.mean()).epsilon(1e-6));
	REQUIRE(skp1.state_cov().rows() == 1);
	REQUIRE(skp1.state_cov().cols() == 1);
	REQUIRE(skp1.gain().rows() == 1);
	REQUIRE(skp1.gain().cols() == 2);
	REQUIRE(skp1.observable() == -1);
	/* z_stats reflects 2*k = z_stats.n_sample() N(0,1) 'random' vars
	 * (since 2 vars per step)
	 * variance of sum (i.e. z_stats.mean() * k) is proportional to k;
	 * variance of mean like 1/k
	 *
	 * kalman filter also should compute covariance estimate like 1/k
	 *
	 */

	REQUIRE(skp1.state_cov()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));
	REQUIRE(skp1.gain()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));
	REQUIRE(skp1.gain()(0, 1) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-6));

	/* estimate at each step should be (approximately)
	 * average of measurements taken so far.
	 * approximate because also affected by prior
	 */

	sk = skp1;
      }

      REQUIRE(sk.state_v()[0] == Approx(10.0).epsilon(1e-2));
      REQUIRE(sk.state_cov()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-3));
      /* result is close but not identical,
       * because initial confidence P0 counts as one sample,
       * so have odd #of samples
       */
      REQUIRE(sk.gain()(0, 0) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-3));
      REQUIRE(sk.gain()(0, 1) == Approx(1.0 / z_stats.n_sample()).epsilon(1e-3));
    } /*TEST_CASE(kalman-identity2)*/

    namespace {
      /* step for kalman filter with:
       * - two state variables x[0], x[1]
       *   x[0] subject to random disturbances,  reverts towards mean 1
       *   x[1] is 1 
       * - process model: x(k+1) = F(k).x(k) + v(k) with
       *    F(k) = | 0.95 0.05 |   v(k) = | v1 |,   v ~ N(0, 0.25)
       *           | 0    1    |          |  0 |
       * - one observation z[0]
       * - coupling matrix: z(k) = H(k).x(k) + w(k), with
       *    H(k) = | 1 0 |
       *
       *    w(k) ~ N(0,1)
       */
      KalmanFilterSpec::MkStepFn
      kalman_revert1_mkstep_fn()
      {
	/* kalman state transition matrix */
	MatrixXd F(2,2);
	F << 0.95, 0.05, 0, 1;

	/* state transition noise */
	MatrixXd Q(2,2);
	Q << 0.0001, 0.0, 0.0, 0.0;

	/* coupling matrix */
	MatrixXd H(1,2);
	H << 1.0, 0.0;

	/* observation errors */
	MatrixXd R(1,1);
	R << 0.25;

	return [F, Q, H, R](KalmanFilterState const & sk,
			    KalmanFilterInput const & zkp1) {
	  KalmanFilterTransition Fk(F, Q);
	  KalmanFilterObservable Hk(H, R);

	  return KalmanFilterStep(sk, Fk, Hk, zkp1);
	}; 
      } /*kalman_revert1_mkstep_fn*/
    } /*namespace*/

    TEST_CASE("kalman-revert1", "[kalmanfilter]") {
      /* like test "kalman-identity",
       * but introduce some system noise.
       */

      constexpr bool c_debug_enabled = false;
      scope lscope("TEST(kalman-revert1)", c_debug_enabled);

      /* seed for rng */
      uint64_t seed = 14950139742636922572UL;

      /* N(0,1) random numbers */
      auto normal_rng = NormalGen<xoshiro256ss>::make(seed,
						      0.0 /*mean*/,
						      1.0 /*sdev*/);

      /* accumulate statistics on observations,
       * use as reference when assessing filter behavior
       */
      SampleStatistics z_stats;

      /* write output to file - use as baseline for regression testing */
      std::string self_test_name = Catch::getResultCapture().getCurrentTestName();

      /* write space-delimited output, suitable for gnuplot
       * omit always-constant values, rely on unit test verifying these
       */
      std::ofstream out(self_test_name);
      out << "step z0 x0 P00 K0" << std::endl;
 
      utc_nanos t0 = Time::ymd_midnight(20220707);

      /* estimate x(0).
       * x(0)[1] is constant 1,  used to achieve mean reversion
       */
      VectorXd x0(2);
      x0 << 1.0 + normal_rng(), 1.0;

      z_stats.include_sample(x0[0]);

      /* kalman prior : Variance = 1, sdev = 1 */
      MatrixXd P0(2,2);
      P0 << 1.0, 0.0, 0.0, 0.0;

      KalmanFilterStateExt s0(0 /*step#*/,
			      t0,
			      x0,
			      P0,
			      MatrixXd::Zero(1, 1) /*K*/,
			      -1 /*j*/);

      auto mk_step_fn
	= kalman_revert1_mkstep_fn();

      KalmanFilterSpec spec(s0, mk_step_fn);
      KalmanFilterStateExt sk = spec.start_ext();

      for(uint32_t i_step = 1; i_step < 100; ++i_step) {
	/* note: for this filter, measurement time doesn't affect behavior */
	utc_nanos tkp1 = sk.tm() + seconds(1);

	VectorXd z(1);
	z << 1.0 + normal_rng();

	z_stats.include_sample(z[0]);

	KalmanFilterInput inputk(tkp1, z);
	KalmanFilterStep step_spec = spec.make_step(sk, inputk);
	KalmanFilterStateExt skp1 = KalmanFilterEngine::step(step_spec);

	if (c_debug_enabled) {
	  lscope.log("filter",
		     xtag("step", i_step),
		     xtag("z", matrix(z)),
		     xtag("x", matrix(skp1.state_v())),
		     xtag("P", matrix(skp1.state_cov())),
		     xtag("K", matrix(skp1.gain())));
	}

	/* headings: step z0 x0 P00 K0 */
	out << i_step
	    << " " << z(0)
	    << " " << skp1.state_v()(0)
	    << " " << skp1.state_cov()(0, 0)
	    << " " << skp1.gain()(0, 0)
	    << "\n";

	REQUIRE(skp1.step_no() == i_step);
	REQUIRE(skp1.tm() == tkp1);
	REQUIRE(skp1.n_state() == 2);
	//
	REQUIRE(skp1.state_v().size() == 2);
	REQUIRE(skp1.state_v()(1) == 1.0);
	//
	REQUIRE(skp1.state_cov().rows() == 2);
	REQUIRE(skp1.state_cov().cols() == 2);
	// test skp1.state_cov()(0,0) vs baseline
	REQUIRE(skp1.state_cov()(0, 0) >= 0.0);
	REQUIRE(skp1.state_cov()(1, 0) == 0.0);
	REQUIRE(skp1.state_cov()(0, 1) == 0.0);
	REQUIRE(skp1.state_cov()(1, 1) == 0.0);
	//
	REQUIRE(skp1.gain().rows() == 2);
	REQUIRE(skp1.gain().cols() == 1);
	REQUIRE(skp1.gain()(0, 0) > 0.0);
	REQUIRE(skp1.gain()(1, 0) == 0.0);
	//
	REQUIRE(skp1.observable() == -1);
	//

	sk = skp1;
      }

      out << std::flush;
      out.close();

      /* compare output with regression baseline.
       * command like:
       *    diff kalman-revert1 utestdata/filter/kalman-revert1
       */
      char cmd_buf[256];
      snprintf(cmd_buf, sizeof(cmd_buf),
	       "diff %s utestdata/filter/%s\n",
	       self_test_name.c_str(),
	       self_test_name.c_str());

      INFO(tostr(self_test_name, xtag("cmd", cmd_buf)));

      std::int32_t err = ::system(cmd_buf);

      REQUIRE(err == 0);
    } /*TEST_CASE(kalman-drift)*/

#ifdef NOT_IN_USE
    namespace {
      /* step for kalman filter with:
       * - two state variables x[0], x[1]
       * - identity process model: x(k+1) = F(k).x(k), with
       *     F(k) = | 1 0 |
       *            | 0 1 |
       * - no process noise
       * - two observations z[0], z[1]
       * - simple coupling matrix: z(k) = H(k).x(k) + w(k), with
       *     H(k) = | 1  0 |
       *            | 0 -1 |
       *   (so sign of z[1] is reversed w.r.t x[1])
       *
       *     w(k) = | w1 |  with w1 ~ N(0,1)
       *            | w2 |
       */
      KalmanFilterSpec::MkStepFn
      kalman_identity2x2_mkstep_fn()
      {
	/* kalman state transition matrix: use identity <-> state is constant */
	MatrixXd F = MatrixXd::Identity(2, 2);

	/* state transition noise: set to 0 */
	MatrixXd Q = MatrixXd::Zero(2, 2);

	/* two direct observations */
	MatrixXd H = MatrixXd::Constant(2 /*#rows*/, 1 /*#cols*/, 1.0 /*M(i,j)*/);
      
	/* observation errors: N(0,1) */
	MatrixXd R = MatrixXd::Identity(2, 2);

	return [F, Q, H, R](KalmanFilterState const & sk,
			    KalmanFilterInput const & zkp1) {
	  KalmanFilterTransition Fk(F, Q);
	  KalmanFilterObservable Hk(H, R);

	  return KalmanFilterStep(sk, Fk, Hk, zkp1);
	};
      } /*kalman_identity2_mkstep_fn*/
    } /*namespace*/
#endif
  } /*namespace ut*/
} /*namespace xo*/

/* end KalmanFilter.test.cpp */

