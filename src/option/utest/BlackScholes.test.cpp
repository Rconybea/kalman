/* @file BlackScholes.test.cpp */

#include "option/BlackScholes.hpp"
#include "logutil/tag.hpp"
#include "catch2/catch.hpp"
#include <array>

namespace xo {
  using xo::option::BlackScholes;
  using xo::option::Greeks;
  using logutil::xtag;

  namespace {
    struct BlackScholesTestCase {
    public:
      BlackScholesTestCase(double K, double S, double s, double r, double t, double exp_tv, double exp_delta, double exp_gamma, double exp_vega, double exp_theta, double exp_rho)
	: strike_{K}, spot_{S}, volatility_{s}, rate_{r}, ttx_{t},
	  exp_tv_{exp_tv}, exp_delta_{exp_delta}, exp_gamma_{exp_gamma},
	  exp_vega_(exp_vega), exp_theta_(exp_theta), exp_rho_(exp_rho) {}

      double strike_;     /*strike*/
      double spot_;       /*spot*/
      double volatility_; /*volatility*/
      double rate_;       /*interest rate*/
      double ttx_;        /*time-to-expiry*/

      double exp_tv_;     /*expected option value. see Greeks.tv*/
      double exp_delta_;  /*expected delta.  see Greeks.delta */
      double exp_gamma_;  /*expected gamma.  see Greeks.gamma */
      double exp_vega_;   /*expected vega.   see Greeks.vega */
      double exp_theta_;  /*expected theta.  see Greeks.theta */
      double exp_rho_;    /*expected rho.    see Greeks.rho */
    }; /*BlackScholesTestCase*/

    using BSTC = BlackScholesTestCase;

    std::array<BlackScholesTestCase, 6> s_test_case_v {
      /* 1: at-the-money options.
       *      .tv increases with ttx
       *      .delta drifts up gently with ttx (b/c forward higher)
       *      .gamma decreases with ttx (spot becomes less informative prior)
       *      .vega  increases with ttx (b/c time-value higher with more time)
       */

      /*     K    S    s    r
       *    ttx              tv     delta      gamma        vega      theta     rho */

      /* 1.1: millisecond option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   1e-9,      3.7847e-6, 0.500002, 4.20522e4, 1.26157e-5, -1.89235e3, 4.99998e-10},
      /* 1.2: 1-day option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   1/365.25, 6.26227e-3, 0.503131, 25.413850, 0.0208738,  -1.1436235, 0.001360353},
      /* 1.3: 1-mo option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   31/365.25,  0.0348561, 0.517428,  4.560247, 0.1161131, -0.2052111, 0.040957509},
      /* 1.4: 3-mo option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   92/365.25,  0.0600095, 0.530005,  2.642162, 0.1996541, -0.1188973, 0.118383471},
      /* 1.5: 1-yr option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   1.0,        0.1192354, 0.5596177, 1.314931, 0.3944793, -0.0591719, 0.440382308},
      /* 1.6: 2-yr option */
      BSTC{1.0, 1.0, 0.3, 0.0,
	   2.0,        0.1679960, 0.583998,  0.919395, 0.5516371, -0.0413728, 0.832004029}
    }; /*s_test_case_v*/
  } /*namespace*/

  namespace ut {
    TEST_CASE("black-scholes-call-pricing", "[blackscholes]") {
      for(size_t i = 0, n = s_test_case_v.size(); i<n; ++i) {
	INFO(xtag("i", i));

	BlackScholesTestCase const & spec = s_test_case_v[i];

	Greeks greeks
	  = BlackScholes::call_greeks(spec.strike_,
				      spec.spot_,
				      spec.volatility_,
				      spec.rate_,
				      spec.ttx_);

	REQUIRE(greeks.tv() == Approx(spec.exp_tv_).epsilon(5e-7));
	REQUIRE(greeks.delta() == Approx(spec.exp_delta_).epsilon(5e-7));
	REQUIRE(greeks.gamma() == Approx(spec.exp_gamma_).epsilon(5e-7));
	REQUIRE(greeks.vega() == Approx(spec.exp_vega_).epsilon(3e-6));
	REQUIRE(greeks.theta() == Approx(spec.exp_theta_).epsilon(5e-7));
	REQUIRE(greeks.rho() == Approx(spec.exp_rho_).epsilon(5e-7));
      }

#ifdef OBSOLETE
      double K = 1.0;  /* strike price $1 */
      double S = 1.0;  /* spot price $1 */
      double s = 0.3;  /* volatility '30% / year' */
      double r = 0.00; /* risk-free rate 5% / year */
      double t = 1e-9; /* time-to-expiry epsilon.   1e-9 years < 1e-6 days ~ 25ms */

      Greeks greeks = BlackScholes::call_greeks(K, S, s, r, t);

      REQUIRE(greeks.tv() >= 0.0);
      REQUIRE(greeks.tv() < 5.0e-6);
#endif
    } /*TEST_CASE(black-scholes-call-pricing)*/
  } /*namespace ut*/
} /*namespace xo*/

/* end BlackScholes.test.cpp */
