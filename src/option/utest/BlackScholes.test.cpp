/* @file BlackScholes.test.cpp */

#include "option/BlackScholes.hpp"
#include "logutil/tag.hpp"
#include "catch2/catch.hpp"
#include <array>

namespace xo {
  using xo::option::BlackScholes;
  using xo::option::Greeks;
  using xo::option::Callput;
  using logutil::xtag;

  namespace {
    struct BlackScholesTestCase {
    public:
      BlackScholesTestCase(Callput pc, double K, double S, double s, double r, double t, double exp_tv, double exp_delta, double exp_gamma, double exp_vega, double exp_theta, double exp_rho)
	: callput_{pc}, strike_{K}, spot_{S}, volatility_{s}, rate_{r}, ttx_{t},
	  exp_tv_{exp_tv}, exp_delta_{exp_delta}, exp_gamma_{exp_gamma},
	  exp_vega_(exp_vega), exp_theta_(exp_theta), exp_rho_(exp_rho) {}

      Callput callput_;   /*call-option or put-option*/
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

    constexpr Callput call = Callput::call;
    constexpr Callput put  = Callput::put;

    std::array<BlackScholesTestCase, 20> s_test_case_v {
      /*
       * 0: eps volatility options.
       * 0.1: call option
       * 0.2: put option
       *
       * 1: at-the-money options.
       *      .very small tv for option with 0 intrinsic value
       *      .very large gamma, since tiny vol means not much time value.
       *        (spot is super strong prior, so small spot changes can affect delta a lot)
       *      .very small theta for same reason
       * 1.1: call option
       *      .tv increases with ttx
       *      .delta drifts up gently with ttx (b/c forward higher -- thx to s^2/2)
       *      .gamma decreases with ttx (spot becomes less informative prior)
       *      .vega  increases with ttx (b/c time-value higher with more time)
       * 1.2: put option
       *      .tv increases with ttx
       *      .delta drifts down gently with ttx (b/c forward higher -- thx to s^2/2)
       *      .gamma same as for call
       *      .vega same as for call
       *      
       */

      /*     pc    K    S     s    r         ttx
	              tv      delta        gamma        vega          theta           rho */

      /* 0.1.1: sub-second option */
      BSTC{call, 1.0, 1.0, 1e-9,  0.0,	    1e-9,
	   1.2656542e-14,  0.500000, 1.261566e13, 1.26157e-5,  -6.30783e-06,    5.0000e-10},
      /* 0.1.2: 1-day option */
      BSTC{call, 1.0, 1.0, 1e-9,  0.0,  1/365.25,
	   2.08744133e-11, 0.500000, 7.6243913e9, 0.02087445, -3.812196e-09,  1.368925e-03},
      /* 0.1.3: 1-mo option */
      BSTC{call, 1.0, 1.0, 1e-9,  0.0, 31/365.25,
	   1.16223975e-10, 0.500000, 1.3693811e9, 0.11622400, -6.846906e-10,  4.243670e-02},
      /* 0.1.4: 3-mo option */
      BSTC{call, 1.0, 1.0, 1e-9,  0.0, 92/365.25,
	   2.00220673e-10, 0.500000, 7.9489774e8, 0.20022065, -3.974489e-10,  1.259411e-01},
      /* 0.1.5: 1-yr option */
      BSTC{call, 1.0, 1.0, 1e-9,  0.0,       1.0,
	   3.98942268e-10, 0.500000, 3.9894228e8, 0.39894229, -1.994711e-10,  5.000000e-01},
      /* 0.1.6: 2-yr option */
      
      /* 0.2.1: sub-second option */
      BSTC{ put, 1.0, 1.0, 1e-9,  0.0,      1e-9,
	    1.2656542e-14, -0.50000, 1.261566e13, 1.26157e-5, -6.307831e-06, -5.000000e-10},
      /* 0.2.2: 1-day option */
      BSTC{ put, 1.0, 1.0, 1e-9,  0.0,  1/365.25,
	    2.0874413e-11, -0.50000, 7.6243913e9, 0.02087445, -3.812196e-09, -1.368925e-03},
      /* 0.2.3: 1-mo option */
      BSTC{ put, 1.0, 1.0, 1e-9,  0.0, 31/365.25,
	    1.1622392e-10, -0.50000, 1.3693811e9, 0.11622400, -6.846906e-10, -4.243670e-02},

      /* TODO: 1-yr option, 2-yr option */

      /* 1.1.1: sub-second option */
      BSTC{call, 1.0, 1.0,  0.3, 0.0,
	   1e-9,      3.7847e-6,   0.500002, 4.20522e4,   1.26157e-5,  -1.89235e3,   4.99998e-10},
      /* 1.1.2: 1-day option */
      BSTC{call, 1.0, 1.0, 0.3, 0.0,
	   1/365.25, 6.26227e-3,   0.503131, 25.413850, 0.0208738,   -1.1436235,   0.001360353},
      /* 1.1.3: 1-mo option */
      BSTC{call, 1.0, 1.0, 0.3, 0.0,
	   31/365.25,  0.0348561,  0.517428,  4.560247, 0.1161131,  -0.2052111,   0.040957509},
      /* 1.1.4: 3-mo option */
      BSTC{call, 1.0, 1.0, 0.3, 0.0,
	   92/365.25,  0.0600095,  0.530005,  2.642162, 0.1996541,  -0.1188973,   0.118383471},
      /* 1.1.5: 1-yr option */
      BSTC{call, 1.0, 1.0, 0.3, 0.0,
	   1.0,        0.1192354,  0.5596177, 1.314931, 0.3944793,  -0.0591719,   0.440382308},
      /* 1.1.6: 2-yr option */
      BSTC{call, 1.0, 1.0, 0.3, 0.0,
	   2.0,        0.1679960,  0.583998,  0.919395, 0.5516371,  -0.0413728,   0.832004029},

      /* 1.2.1: millisecond option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	   1e-9,      3.7847e-6,  -0.499998, 4.20522e4, 1.26157e-5, -1.89235e3,  -5.00002e-10},
      /* 1.2.2: 1-day option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	    1/365.25, 6.26227e-3, -0.496869, 25.413850, 0.0208738,  -1.1436235,  -0.001377498},
      /* 1.2.3: 1-mo option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	   31/365.25,  0.0348561, -0.482572,  4.560247, 0.1161131,  -0.2052111,  -0.043915865},
      /* 1.2.4: 3-mo option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	   92/365.25,  0.0600095, -0.4699952, 2.642162, 0.1996541,  -0.1188973,  -0.133498802},
      /* 1.2.5: 1-yr option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	   1.0,        0.1192354, -0.4403823, 1.314931, 0.3944793,  -0.0591719,  -0.559617692},
      /* 1.2.6: 2-yr option */
      BSTC{ put, 1.0, 1.0, 0.3, 0.0,
	   2.0,        0.1679960, -0.4160020,  0.919395, 0.5516371,  -0.0413728, -1.167995971}
    }; /*s_test_case_v*/

  } /*namespace*/

  namespace ut {
    TEST_CASE("black-scholes-call-pricing", "[blackscholes]") {
      for(size_t i = 0, n = s_test_case_v.size(); i<n; ++i) {
	INFO(xtag("i", i));

	BlackScholesTestCase const & spec = s_test_case_v[i];

	Greeks greeks
	  = BlackScholes::greeks(spec.callput_,
				 spec.strike_,
				 spec.spot_,
				 spec.volatility_,
				 spec.rate_,
				 spec.ttx_);

	char buf[100];
	snprintf(buf, sizeof(buf), "tv=%.7e delta=%.7e gamma=%.7e theta=%.7e rho=%.7e", greeks.tv(), greeks.delta(), greeks.gamma(), greeks.theta(), greeks.rho());
	std::cout << buf << std::endl;

	REQUIRE(greeks.tv() == Approx(spec.exp_tv_).epsilon(5e-7));
	REQUIRE(greeks.delta() == Approx(spec.exp_delta_).epsilon(5e-7));
	REQUIRE(greeks.gamma() == Approx(spec.exp_gamma_).epsilon(5e-7));
	REQUIRE(greeks.vega() == Approx(spec.exp_vega_).epsilon(3e-6));
	REQUIRE(greeks.theta() == Approx(spec.exp_theta_).epsilon(5e-7));
	REQUIRE(greeks.rho() == Approx(spec.exp_rho_).epsilon(5e-7));

	/* check put-call parity
	 * we have:  buying call + selling put at (K = strike, ttx = expiry)
	 * is the same as buying a forward to exchange K for underlying at time ttx.
	 *
	 * this implies:
	 *    C - P = D(F - K) = S - D.K    (since S = D.F)
	 * where
	 *    C = call value
	 *    P = put value
	 *    D = discount factor exp(-r.t)
	 *    F = forward price
	 *    K = strike
	 */
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
