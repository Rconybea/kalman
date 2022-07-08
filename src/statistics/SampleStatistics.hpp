/* @file SampleStatistics.hpp */

#pragma once

#include <cstdint>

namespace xo {
  namespace statistics {
    /* accumlate statistics online for a sample */
    class SampleStatistics {
    public:
      SampleStatistics() = default;

      /* given we have a sample S(n) of size n with given mean,
       * compute mean of sample with one event x added
       *
       * n.  #of samples *preceding* x
       */
      static double update_online_mean(double x, uint32_t n, double mean) {
        /* to update mean in a numerically stable way:
         * avoid computing running sample sum,  to avoid
         * adding floating point numbers with distant magnitudes;
         * instead compute correction to the mean directly
         *
         *                n  /  x(i) \
         *   mean(Sn) := Sum | ----- |
         *               i=1 \   n   /
         *
         * so
         *                  n+1 /  x(i) \
         *   mean(S(n+1)) = Sum | ----- |
         *                  i=1 \  n+1  /
         *
         *                   n  n+1 /  x(i) \
         *                = --- Sum | ----- |
         *                  n+1 i=1 \   n   /
         *
         *                   n  / x(n+1)    n  x(i) \
         *                = --- | ------ + Sum ---- |
         *                  n+1 \   n      i=1  n   /
         *
         *                  x(n+1)   /  n               \
         *                = ------ + | --- . mean(S(n)) |
         *                   n+1     \ n+1              /
         *
         *                  x(n+1)                / -1               \
         *                = ------ + mean(S(n)) + | --- . mean(S(n)) |
         *                   n+1                  \ n+1              /
         *
	 *                = mean(S(n)) + (x(n+1) - mean(S(n))) / (n+1)
	 */
	return mean + ((1.0 / (n+1)) * (x - mean));
      } /*update_online_mean*/

      /*
       * with S(n) = Sn = {set of n samples},
       *      u(n) = mean(Sn)
       *
       * (with mean, variance meaning "estimate for")
       *
       *                    1     n  /      2 \   /  1             \ 2
       *   variance(Sn) := --- . Sum | (x(i)  | - | --- . Sum x(i) |
       *                    n    i=1 \        /   \  n    i=1      /
       *
       * using Welford's recurrence for 2nd moment:
       *
       * define
       *    M2(n+1) := M2(n) + (x(n+1) - mean(S(n)))
       *                        . (x(n+1) - mean(S(n+1))
       *
       * then unbiased variance estimate for S(n+1) is:
       *
       *    M2(n+1)
       *    -------
       *       n
       *
       * x.          new sample value
       * mean_np1.   mean estimate for S(n+1)
       * mean_n.     mean estimate for S(n)
       * moment2.    2nd moment for S(n)
       */
      static double update_online_moment2(double x, 
					  double mean_np1, double mean_n,
					  double moment2)
      {
	return moment2 + (x - mean_n) * (x - mean_np1);
      } /*update_online_moment2*/

      uint32_t n_sample() const { return n_sample_; }
      double mean() const { return mean_; }
      double moment2() const { return moment2_; }
      /* 'sample variance' = variance estimate,
       * applying Bessel correction for sample bias
       *
       * require: n_sample >= 2
       */
      double sample_variance() const { return moment2_ / (n_sample_ - 1); }

      /* biased variance estimate
       *   = (1 - 1/(n+1)) * .sample_variance()
       *
       * .variance() -> .sample_variance() as sample size -> +oo
       *
       * require: n_sample >= 1
       */
      double variance() const { return moment2_ / n_sample_; }

      void include_sample(double x) {
	/* n+1 */
	uint32_t np1 = this->n_sample_ + 1;
	
	double mean_np1    = update_online_mean(x, this->n_sample_, this->mean_);
	double moment2_np1 = update_online_moment2(x, this->mean_, mean_np1, this->moment2_);

	this->n_sample_ = np1;
	this->mean_     = mean_np1;
	this->moment2_  = moment2_np1;
      } /*include_sample*/

    private:
      uint32_t n_sample_ = 0;
      /* estimated mean */
      double mean_ = 0.0;
      /* estimated 2nd moment E[X^2] */
      double moment2_ = 0.0;
    }; /*SampleStatistics*/
  } /*namespace statistics*/
} /*namespace xo*/

/* end SampleStatistics.hpp */
