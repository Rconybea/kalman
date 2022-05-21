/* @file Histogram.hpp */

#pragma once

#include "statistics/SampleStatistics.hpp"
#include <vector>
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

      /* add one sample, x, to this bucket */
      void include_sample(double x) {
	int n = this->n_sample_;

	this->n_sample_ = n+1;
	this->sum_ += x;
	this->mean_ = SampleStatistics::update_online_mean(x, n, this->mean_);
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

      uint32_t n_bucket() const { return n_interior_bucket_ + 2; }

      double bucket_width() const { return (this->hi_bucket_ - this->lo_bucket_) / this->n_interior_bucket_; }

      const_iterator begin() const { return bucket_v_.begin(); }
      const_iterator end() const { return bucket_v_.end(); }
      Bucket const & lookup(uint32_t ix) const { return this->bucket_v_[ix]; }

      double bucket_hi_edge(uint32_t ix) {
	if(ix < n_interior_bucket_ + 1)
	  return this->lo_bucket_ + ix * this->bucket_width();
	else
	  return std::numeric_limits<double>::infinity();
      } /*bucket_hi_edge*/
      
      /* index (into .bucket_v[]) of bucket to use for a sample with value x */
      uint32_t bucket_ix(double x) {
	if(x < this->lo_bucket_)
	  return 0;

	if(x < this->hi_bucket_)
	  return 1 + static_cast<uint32_t>((x - this->lo_bucket_) / this->bucket_width());

	return this->n_interior_bucket_ + 1;
      } /*bucket_ix*/

      void include_sample(double x) {
	uint32_t ix = this->bucket_ix(x);

	this->bucket_v_[ix].include_sample(x);
      } /*include_sample*/

    private:
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
