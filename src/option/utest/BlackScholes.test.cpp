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
      BlackScholesTestCase(double K, double S, double s, double r, double t, double exp_tv)
	: strike_{K}, spot_{S}, volatility_{s}, rate_{r}, ttx_{t},
	  exp_tv_{exp_tv} {}

      double strike_;     /*strike*/
      double spot_;       /*spot*/
      double volatility_; /*volatility*/
      double rate_;       /*interest rate*/
      double ttx_;        /*time-to-expiry*/

      double exp_tv_;     /*expected option value*/
    }; /*BlackScholesTestCase*/

    using BSTC = BlackScholesTestCase;

    std::array<BlackScholesTestCase, 1> s_test_case_v {
      BSTC{1.0, 1.0, 0.3, 0.0, 1e-9, 3.7847e-6}
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
