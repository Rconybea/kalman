/* @file Exponential.hpp */

#pragma once

#include "distribution/Distribution.hpp"
#include <limits>
#include <cmath>

namespace xo {
  namespace distribution {
    /* Exponential probability distribution */
    class Exponential : public Distribution<double> {
    public:
      explicit Exponential(double lm) : lambda_(lm) {}

      /* exponential probability density:
       *
       *                    -lm.x
       *    p(x) = {  lm . e      ,  x > 0
       *           {  0           ,  x <= 0
       *
       */
      static double density_impl(double lambda, double x) {
	if(x <= 0.0)
	  return 0.0;

	return lambda * ::exp(-lambda * x);
      } /*density_impl*/

      /* exponential distribution:
       *
       *                -lm.x
       *    F(x) = 1 - e         , x > 0
       *    F(x) = 0             , x <= 0
       */
      static double distr_impl(double lambda, double x) {
	if(x <= 0.0)
	  return 0.0;

	return 1.0 - ::exp(-lambda * x);
      } /*distr_impl*/

      /* compute x: F(x)=y,  where F(x)
       * is the cumulative exponential probability distributio.
       *
       *                -lm.x
       *    F(x) = 1 - e        , x>0
       *
       *     -lm.x
       *    e      = 1 - F(x)
       *
       *    -lm.x  = ln(1 - F(x))
       *    
       *    x      = -ln(1 - F(x)) / lm,   0 < F(x) < 1
       */
      static double distr_inverse_impl(double lambda, double Fx) {
	if(Fx < 0.0)
	  return -1.0 * std::numeric_limits<double>::infinity();
	if(Fx >= 1.0)
	  return +1.0 * std::numeric_limits<double>::infinity();

	return (-1.0 / lambda) *  ::log(1.0 - Fx);
      } /*distr_inverse_impl*/

      double density(double x) const {
	return density_impl(this->lambda_, x);
      } /*density*/

      double distribution(double x) const {
	return distr_impl(this->lambda_, x);
      } /*distribution*/

      double distribution_inverse(double y) const {
	return distr_inverse_impl(this->lambda_, y);
      } /*distribution_inverse*/

      // ----- inherited from Distribution<double> -----

      virtual double cdf(double const & x) const override {
	return distribution(x);
      } /*cdf*/

    private:
      /* intensity parameter.
       * require: lambda > 0
       */
      double lambda_;
  } /*namespace distribution*/
} /*namespace xo*/

/* end Exponential.cpp */
