/* @file KolmogorovSmirnov.hpp */

#pragma once

#include "distribution/Distribution.hpp"
#include "logutil/scope.hpp"
#include <cmath>
#include <cstdint>

namespace xo {
  /* Kolmogorov-Smirnov probability distribution */
  namespace distribution {
    
    class KolmogorovSmirnov : public Distribution<double> {
    public:
      KolmogorovSmirnov() = default;

      /* kolmogorov-smirnov (KS) cumulative distribution
       *
       * cdf is defined by the series:
       *
       *                 +oo /     j          2  2  \
       *   P1(x) = 1 + 2 Sum | (-1)  .exp(-2.j .x ) |
       *                 j=1 \                      /
       *
       * this converges rapidly for x > 1.  expanding the first few terms:
       *
       *                  /          2            2  2            2  2            2  2  \
       *   P1(x) ~= 1 + 2.| -exp(-2.x ) + exp(-2.2 .x ) - exp(-2.3 .x ) + exp(-2.4 .x ) |
       *                  \                                                            /
       *
       *                  /          2            2 4          2 9           2 16 \
       *          = 1 + 2.| -exp(-2.x ) + exp(-2.x )  - exp(2.x )  + exp(-2.x )   |
       *                  \                                                      /
       *
       *                  /            4       9       16 \
       *          = 1 + 2.| T(x) + T(x)  + T(x)  + T(x)   |
       *                  \                               /
       *
       * with T(x) = .term_aux(1, x)
       *
       * with x=1:
       *   term1_aux(1, 1): -0.135335
       *   term1_aux(2, 1):  3.35463e-4
       *   term1_aux(3, 1): -1.52300e-8
       *   term1_aux(4, 1):  1.26642e-14
       *   term1_aux(5, 1): -1.92875e-22
       *
       * with x=1.18:
       *   term1_aux(1, 1): -0.0617414
       *   term1_aux(2, 1):  1.45314e-5
       *   term1_aux(3, 1): -1.30374e-11
       *   term1_aux(4, 1):  4.45890e-20
       *   term1_aux(5, 1): -5.83124e-31
       *
       * ----------------------------------------------------------------
       *
       * There's an alternative series for the KS distribution,
       * that converges rapidly for small x (x < ~1.18)
       *
       *                                  /           2   2 \
       *           sqrt(2.pi)   +oo /     |   (2j - 1) .pi  | \
       *   P2(x) = ---------- . Sum | exp | - ------------  | |
       *               x        j=1 \     |          2      | /
       *                                  \       8.x       /
       *
       *                 /     2  \
       *                 |   pi   |
       * witu U(x) = exp | - ---- |
       *                 |      2 |
       *                 \   8.x  /
       * we have
       *                            /             2 \
       *           sqrt(2.pi)   +oo |     (2j - 1)  |
       *   P2(x) = ---------- . Sum | U(x)          |
       *               x        j=1 |               |
       *                            \               /
       *
       *                        /                                   \
       *           sqrt(2.pi)   |            9       25      49     |
       *         = ---------- . | U(x) + U(x)  + U(x)  + U(x)  + .. |
       *               x        |                                   |
       *                        \                                   /
       *
       * with x=1.0:
       *   term2_aux(1, 1): 0.291213
       *   term2_aux(2, 1): 1.50625e-5
       *   term2_aux(3, 1): 4.02964e-14
       *   term2_aux(4, 1): 5.57599e-27
       *
       * with x=1.18:
       *   term2_aux(1, 1.18): 0.412292
       *   term2_aux(2, 1.18): 3.44223e-4
       *   term2_aux(3, 1.18): 2.39945e-10
       *   term2_aux(4, 1.18): 1.39652e-19
       */
      static double term1_aux(uint32_t j, double x) {
	double f = ::exp(-2.0 * x * x);
	/* sgn:
	 *  +1 for j in {2, 4, 6, ..}
	 *  -1 for j in {1, 3, 5, ..}
	 */
	int32_t sgn = (((j & 0x1) == 0) ? +1 : -1);

	if(j == 1) {
	  return sgn * f;
	} else {
	  return sgn * ::pow(f, j*j);
	}
      } /*term1_aux*/	
      
      /* implements P1(x) above;  truncating at 4 terms.
       * use .distr1() for x > ~1.18
       */
      static double distr1_impl(double x) {
	double   f = term1_aux(1.0, x);
	double  f2 = f*f;   /*f^2*/
	double  f4 = f2*f2; /*f^4*/
	double  f8 = f4*f4; /*f^8*/
	double  f9 = f8*f;  /*f^9*/
	double f16 = f8*f8; /*f^16*/

	double   r = ((f16 + f9) + f4) + f;

	return 1.0 + 2.0*r;
      } /*distr1_impl*/

      /* pi: 3.141592... */
      static constexpr double c_pi = M_PI;
      /* pi^2 / 8 */
      static constexpr double c_pi2_8 = 0.125 * c_pi * c_pi;

      /* computes
       *
       *       (2j-1)^2
       *   U(x)
       *
       * require:
       * - j >= 1
       */
      static double term2_aux(uint32_t j, double x) {
	double u = ::exp(-c_pi2_8 / (x * x));

	if(j == 1) {
	  return u;
	} else {
	  uint32_t j2m1 = 2*j - 1;

	  return ::pow(u, j2m1 * j2m1);
	}
      } /*term2_aux*/

      /* implements P2(x) above; truncating at 4 terms */
      static double distr2_impl(double x) {
	static double c_sqrt_2pi
	  = ::sqrt(2.0 * c_pi);

	double scale = c_sqrt_2pi / x;

	double  u = term2_aux(1, x);
	double u2 = u*u;        /*u^2*/
	double u4 = u2*u2;      /*u^4*/
	double u8 = u4*u4;      /*u^8*/
	double u16 = u8*u8;     /*u^16*/
	double u32 = u16*u16;   /*u^32*/

	double  u9 = u8*u;      /*u^9*/
	double u25 = u16*u8*u;  /*u^25*/
	double u49 = u32*u16*u; /*u^49*/
	
	double r = ((u49 + u25) + u9) + u;

	return scale * r;

      } /*distr2_impl*/
	
      static double distr_impl(double x) {
	using logutil::tostr;

	constexpr char const * c_self = "KolmogorovSmirnov::distr_impl";

	if(x < 0.0)
	  throw std::runtime_error(tostr(c_self, "KS(x) cdf defined for x>=0"));

	if(x == 0.0)
	  return 0;

	if(x < 1.18) {
	  /* P2(x) converges fastest */
	  return distr2_impl(x);
	} else {
	  /* P1(x) converges fastest */
	  return distr1_impl(x);
	}
      } /*distr_impl*/

      /* p-value for significance of a particular value of the KS-statistic
       * obtain this statistic for a sample distribution using
       *   Empirical.ks_stat_1sided(dexp)
       * for some target distribution dexp
       *
       * ne.
       *   for 1-sided test:  #of points in sample, i.e. Empirical.n_sample()
       *   for 2-sided test:  (n1 . n2) / (n1 + n2)
       * D.
       *   max difference between observed and expected cumulative distributions.
       *   see Empirical.ks_stat_1sided()
       */
      static double ks_pvalue(uint32_t ne, double D)
      {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "KolmogorovSmirnov::ks_pvalue";
	constexpr bool logging_enabled_flag = false;

	scope lscope(c_self, logging_enabled_flag);

	double ne_sqrt = ::sqrt(ne);

	/* argument to KS-distribution.
	 *   x in [0, +oo)
	 *
	 * A large value for x -> small value for 1 - KSdist(x),
	 * i.e. represents probability that a sample of size ne with a 
	 * KS-statistic of magnitude D or larger,  could be drawn from
	 * distribution dexp.
	 */
	double x = ne_sqrt + 0.12 + (0.11 / ne_sqrt);

	double xD = x * D;

	double pvalue = 1.0 - distr_impl(xD);

	if(logging_enabled_flag)
	  lscope.log(xtag("ne", ne),
		     xtag("D", D),
		     xtag("ne_sqrt", ne_sqrt),
		     xtag("x", x),
		     xtag("xD", xD),
		     xtag("pvalue", pvalue));

	return pvalue;
      } /*ks_pvalue*/

      /* cumulative distribution function */
      double distribution(double x) const {
	return distr_impl(x);
      } /*distribution*/

      // ----- inherited from Distribution<double> -----
      virtual double cdf(double const & x) const {
	return this->distribution(x);
      } /*cdf*/
    }; /*KolmogorovSmirnov*/
  } /*namespace distribution*/
} /*namespace xo*/

/* end KolmogorovSmirnov.hpp */
