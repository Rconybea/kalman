/* @file Histogram.hpp */

#pragma once

#include "statistics/SampleStatistics.hpp"
#include "logutil/scope.hpp"
#include <vector>
#include <cmath>
#include <cstdint>

namespace xo {
  namespace statistics {
    /* sample statistics for a histogram bucket */
    class Bucket {
    public:
      Bucket() = default;

      uint32_t n_sample() const { return n_sample_; }
      double sum() const { return sum_; }
      double mean() const { return mean_; }
      double sample_variance() const { return (n_sample_ > 1) ? moment2_ / (n_sample_ - 1) : 0.0; }
      double standard_error() const { return ::sqrt(this->sample_variance()); }

      /* to estimate standard error of the mean:
       * 0. let nk = .n_sample be the #of samples falling into this bin.
       *    n is the total #of samples across all bins.
       *    (i.e. Histogram.n_sample)
       * 1. imagine probability of a sample falling in this bin
       *    is the observed frequency p = (.n_sample / n)
       * 2. imagine a Bernoulli random variable Bp(i) associated with each sample x(i)
       *    {1, with probability p;  0 with probability q=1-p})
       * 3. each Bp(i) has mean p, variance p(1-p)
       * 4. sum of the Bp(1) .. Bp(n) has mean n.p = nk,
       *    variance
       *       n.p.(1-p)
       *     = n.(nk/n).(1 - nk/n)
       *     = nk.(1 - nk/n)
       *    (by central limit theorem we can treat this as approximately normal
       *     for sufficiently large n)
       * 5. standard error of Sum{Bp(i)}
       *    will be
       *       sqrt(nk.(1 - nk/n))
       */
      double n_sample_stderr(uint32_t n) const {
	double nr = 1.0 / n;
	uint32_t nk = this->n_sample_;

	return ::sqrt(nk * (1.0 - nk * nr));
      } /*n_sample_stderr*/
      
      /* add one sample, x, to this bucket */
      void include_sample(double x) {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "Bucket::include_sample";
	constexpr bool c_logging_enabled = false;

	int n = this->n_sample_;

	this->n_sample_ = n+1;
	this->sum_ += x;

	double mean_n = this->mean_;
	double mom2_n = this->moment2_;
	double mean_np1 = SampleStatistics::update_online_mean(x, n, mean_n);
	double mom2_np1 = SampleStatistics::update_online_moment2(x,
								  mean_np1, mean_n,
								  mom2_n);
	scope lscope(c_self, c_logging_enabled);
	if(c_logging_enabled) {
	lscope.log("update",
		   xtag("x", x), xtag("n", n),
		   xtag("sum", sum_),
		   xtag("mean(n)", mean_n),
		   xtag("mom2(n)", mom2_n),
		   xtag("mean(n+1)", mean_np1),
		   xtag("mom2(n+1)", mom2_np1));
	}

	this->mean_ = mean_np1;
	this->moment2_ = mom2_np1;
      } /*include_sample*/

    private:
      /* #of samples in this bucket (will be #of times .sample() has been called) */
      uint32_t n_sample_ = 0;
      /* sum of samples in this bucket */
      double sum_ = 0.0;
      /* mean of values in this bucket
       * -- use online algo to avoid catastrophic errors for large #samples
       */
      double mean_ = 0.0;
      double moment2_ = 0.0;
    }; /*Bucket*/

    /* accumulate histogram on sampled data */
    class Histogram {
    public:
      using const_iterator = std::vector<Bucket>::const_iterator;

    public:
      Histogram(uint32_t n_interior_bucket, double lo_bucket, double hi_bucket)
	: n_interior_bucket_(n_interior_bucket),
	  lo_bucket_(lo_bucket),
	  hi_bucket_(hi_bucket),
	  bucket_v_(n_interior_bucket + 2)
      {}

      uint32_t n_sample() const { return n_sample_; }
      uint32_t n_bucket() const { return n_interior_bucket_ + 2; }

      double bucket_width() const { return (this->hi_bucket_ - this->lo_bucket_) / this->n_interior_bucket_; }

      const_iterator begin() const { return bucket_v_.begin(); }
      const_iterator end() const { return bucket_v_.end(); }
      Bucket const & lookup(uint32_t ix) const { return this->bucket_v_[ix]; }

      double bucket_hi_edge(uint32_t ix) const {
	if(ix < n_interior_bucket_ + 1)
	  return this->lo_bucket_ + ix * this->bucket_width();
	else
	  return std::numeric_limits<double>::infinity();
      } /*bucket_hi_edge*/
      
      /* index (into .bucket_v[]) of bucket to use for a sample with value x */
      uint32_t bucket_ix(double x) const {
	if(x < this->lo_bucket_)
	  return 0;

	if(x < this->hi_bucket_)
	  return 1 + static_cast<uint32_t>((x - this->lo_bucket_) / this->bucket_width());

	return this->n_interior_bucket_ + 1;
      } /*bucket_ix*/

      void include_sample(double x) {
	uint32_t ix = this->bucket_ix(x);

	++(this->n_sample_);
	this->bucket_v_[ix].include_sample(x);
      } /*include_sample*/

    private:
      /* #of samples across all buckets */
      uint32_t n_sample_ = 0;
      /* #of interior buckets:  split [.lo_bucket, .hi_bucket] into
       * equally-spaced intervals of width (.hi_bucket - .lo_bucket) / .n_bucket
       */
      uint32_t n_interior_bucket_ = 0;
      /* right edge of first bucket (left edge is -oo) */
      double lo_bucket_ = 0.0;
      /* left edge of last bucket (right edge is +oo) */
      double hi_bucket_ = 0.0;

      /* hisogram buckets */
      std::vector<Bucket> bucket_v_;
    }; /*Histogram*/
  } /*namespace statistics*/
} /*namespace xo*/

/* end Histogram.hpp */
