/* @file Normal.hpp */

#pragma once

#include <cmath>

namespace xo {
  namespace distribution {
    /* TODO: inherit Distribution<double> */
    class Normal {
    public:
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

      /* TODO: implement .cdf() */
    }; /*Normal*/
  } /*namespace distribution*/
} /*namespace xo*/

/* end Normal.hpp */
