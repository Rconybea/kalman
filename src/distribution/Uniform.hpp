/* @file Uniform.hpp */

#pragma once

#include "distribution/Distribution.hpp"

namespace xo {
  namespace distribution {
    /* uniform distribution on an interval */
    class Uniform : public Distribution<double> {
    public:
      Uniform(double lo, double hi) : lo_(lo), hi_(hi) {}

      static Uniform unit() { return Uniform(0.0, 1.0); }

      static double density_impl(double lo, double hi) {
	return 1.0 / (hi - lo);
      } /*density_impl*/

      static double distr_impl(double lo, double hi, double x) {
	return (x - lo) / (hi - lo);
      } /*distr_impl*/

      double lo() const { return lo_; }
      double hi() const { return hi_; }

      double density(double /*x*/) const {
	return density_impl(this->lo_, this->hi_);
      } /*density*/

      double distribution(double x) const {
	return distr_impl(this->lo_, this->hi_, x);
      } /*distribution*/

      // ----- inherited from Distribution<double> -----

      virtual double cdf(double const & x) const override {
	return distribution(x);
      } /*cdf*/

    private:
      /* Invariant: .lo < .hi */
      double lo_;
      double hi_;
    }; /*Uniform*/
  } /*namespace distribution*/
} /*namespace xo*/
