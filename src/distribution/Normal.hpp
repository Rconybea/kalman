/* @file Normal.hpp */

#pragma once

#include "distribution/Distribution.hpp"
#include <cmath>

namespace xo {
  namespace distribution {
    /* the guassian distribution,  with mean 0 and variance 1
     */
    class Normal : public Distribution<double> {
    public:
      Normal() = default;

      /* normal probability density:
       *
       *                  x^2
       *            -(1/2)              1/2
       *    p(x) = e            / (2.pi)
       */
      static double density(double x) {
	static double c_sqrt_2pi = ::sqrt(2 * M_PI);

	return ::exp(-0.5 * x * x) / c_sqrt_2pi;
      } /*density*/

      /* cumulative distribution function for N(0,1):
       *    
       *   / x
       *   |
       *   |    p(x).dx
       *   |
       *   / -oo 
       *
       * where p(x) is the normal density function p(x) = e^[-x^2/2]
       */
      static double cdf_impl(double x) {
	return 0.5 * std::erfc(-M_SQRT1_2 * x);
      } /*cdf_impl*/

      // ----- inherited from Distribution<double> -----

      virtual double cdf(double const & x) const override {
	return cdf_impl(x);
      } /*cdf*/
    }; /*Normal*/
  } /*namespace distribution*/
} /*namespace xo*/

/* end Normal.hpp */
