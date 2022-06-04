/* @file Empirical.hpp */

#pragma once

#include "distribution/Distribution.hpp"
#include "tree/RedBlackTree.hpp"
#include <map>
#include <cstdint>

namespace xo {
namespace distribution {

  /* representation for counter,
   * recording #of samples with the same value
   */
  using CounterRep = uint32_t;

  /* counter;  for use with Empirical distribution below
   */
  class Counter {
  public:
    Counter() = default;
    Counter(CounterRep n) : count_(n) {}
    
    CounterRep count() const { return count_; }

    void incr() { ++count_; }

    operator CounterRep () const { return count_; }

    Counter & operator+=(CounterRep n) { count_ += n; return *this; }
    
  private:
    CounterRep count_ = 0;
  }; /*Counter*/

  /* an empirical distribution over a given domain
   * (e.g. double as proxy for IR),
   * obtained by sorting equally-weighted samples
   */
  template<typename Domain>
  class Empirical : public Distribution<Domain> {
  public:
    using SampleMap = xo::tree::RedBlackTree<Domain,
					     Counter,
					     xo::tree::SumReduce<uint32_t>>;
    using const_iterator = typename SampleMap::const_iterator;

  public:
    Empirical() = default;

    uint32_t n_sample() const { return n_sample_; }
    const_iterator begin() const { return sample_map_.begin(); }
    const_iterator end() const { return sample_map_.end(); }

    /* compute kolmogorov-smirnov statistic with a non-sampled distribution.
     * if d2 is sampled,  should use .ks_stat_2sided() instead
     */
    double ks_stat_1sided(Distribution<Domain> const & d2) const {
      double ks_stat = 0.0;

      /* for i'th loop iteration below:
       *   xj_sum = sum of all x[j] with j<=i
       */
      uint32_t xj_sum = 0;

      /* #of sample in this distribution,  as double */
      double nr = 1.0 / this->n_sample();

      /* loop over elements x[i] of this (sampled) distribution,
       * compare cdf(x[i]) with d2.cdf(x[i])
       *
       * KS stat is the maximum observed difference.
       */
      for(auto const & point : this->sample_map_) {
	Domain const & xi = point.first;
	uint32_t xi_count = point.second;

	xj_sum += xi_count;

	/* p1 = xi_sum / n1,  where n1 = .n_sample() */
	double p1 = xj_sum * nr;
	double p2 = d2.cdf(xi);

	double dp = std::abs(p1 - p2);

	ks_stat = std::max(ks_stat, dp);
      }

      return ks_stat;
    } /*ks_stat_1sided*/

    /* introduce one new sample into this distribution */
    void include_sample(Domain const & x) {
      ++(this->n_sample_);

      /* note: xo::tree::RedBlackTree doesn't provide the usual reference result
       *       from operator[];  it needs to intervene after assignment to update
       *       order statistics
       */
      auto lhs = this->sample_map_[x];

      lhs += 1;
    } /*include_sample*/

    // ----- inherited from Distribution<Domain> -----

    virtual double cdf(Domain const & x) const override {
      /* computes #of samples with values <= x */
      uint32_t nx = this->sample_map_.reduce_lub(x, true /*is_closed*/);
      size_t n = this->n_sample();

      return static_cast<double>(nx) / n;
    } /*cdf*/

  private:
    /* count #of calls to .include_sample() */
    CounterRep n_sample_ = 0;

    /* .sample_map_[x] counts the #of times
     * .include_sample(x) has been called.
     */
    SampleMap sample_map_;
  }; /*Empirical*/

} /*namespace distribution*/
} /*namespace xo*/

/* end Empirical.hpp */
