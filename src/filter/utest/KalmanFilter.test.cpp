/* @file KalmanFilter.test.cpp */

#include "filter/KalmanFilter.hpp"
#include "statistics/SampleStatistics.hpp"
#include "random/Normal.hpp"
#include "random/xoshiro256.hpp"
#include "logutil/scope.hpp"
#include <catch2/catch.hpp>

namespace xo {
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

      /* for this filter,  transition + observable stages are constant */
      KalmanFilterTransition Fk
	= KalmanFilterTransition(F, Q);
      KalmanFilterObservable Hk
	= KalmanFilterObservable(H, R);

      KalmanFilterState s0(0 /*step#*/,
			   t0,
			   x0,
			   P0);

      KalmanFilterStateExt sk(s0.step_no(),
			      s0.tm(),
			      s0.state_v(),
			      s0.state_cov(),
			      MatrixXd::Zero(1, 1) /*K*/);

      for(uint32_t i_step = 1; i_step < 100; ++i_step) {
	/* note: for this filter,  measurement time doesn't matter */
	utc_nanos tkp1 = sk.tm() + seconds(1);

	VectorXd z(1);
	z << 10.0 + normal_rng();

	INFO(tostr("z=", z));

	z_stats.include_sample(z[0]);

	KalmanFilterInput inputk
	  = KalmanFilterInput(z);

	KalmanFilterStateExt skp1
	  = KalmanFilterEngine::step(tkp1,
				     sk,
				     Fk,
				     Hk,
				     inputk);

	REQUIRE(skp1.step_no() == i_step);
	REQUIRE(skp1.tm() == tkp1);
	REQUIRE(skp1.n_state() == 1);
	REQUIRE(skp1.state_v().size() == 1);
	REQUIRE(skp1.state_v()[0] == Approx(z_stats.mean()).epsilon(1e-6));
	REQUIRE(skp1.state_cov().rows() == 1);
	REQUIRE(skp1.state_cov().cols() == 1);
	REQUIRE(skp1.gain().rows() == 1);
	REQUIRE(skp1.gain().cols() == 1);

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
  } /*namespace ut*/
} /*namespace xo*/

/* end KalmanFilter.test.cpp */

