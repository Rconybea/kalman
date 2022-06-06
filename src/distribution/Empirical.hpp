/* @file Empirical.hpp */

#pragma once

#include "distribution/Distribution.hpp"
#include "tree/RedBlackTree.hpp"
#include "logutil/scope.hpp"
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
    std::pair<double, double> ks_stat_1sided(Distribution<Domain> const & d2) const {
      using logutil::scope;
      using logutil::xtag;

      constexpr char const * c_self = "Empirical::ks_stat_1sided";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

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

	if(c_logging_enabled)
	  lscope.log(c_self,
		   xtag("xi", xi),
		   xtag("xi_count", xi_count),
		   xtag("xj_sum", xj_sum),
		   xtag("p1", p1),
		   xtag("p2", p2),
		   xtag("dp", dp));

	ks_stat = std::max(ks_stat, dp);
      }

      return std::pair<double, double>(this->n_sample(), ks_stat);
    } /*ks_stat_1sided*/

    /* compute kolmogorov-smirnov statistic with a sampled distribution;
     * assess likelihood that both samples come from the same population.
     */
    std::pair<double, double> ks_stat_2sided(Empirical<Domain> const & d2) {
      /* loop once over both sample distributions;
       * algorithm is O(n1 + n2) for two empirical
       * distributions with n1,n2 points respectively
       */

      /* return value observed here */
      double ks_stat = 0.0;

      auto     ix1 = this->sample_map_.begin();
      auto end_ix1 = this->sample_map_.end();

      auto     ix2 = d2.sample_map_.begin();
      auto end_ix2 = this->sample_map_.end();

      uint32_t xj1_sum = 0;
      uint32_t xj2_sum = 0;

      uint32_t n1 = this->n_sample();
      uint32_t n2 = d2.n_sample();

      double nr1 = 1.0 / n1;
      double nr2 = 1.0 / n2;

      /*  ^
       * 1|                   **.
       *  |                ...*..
       *  |                .  *
       *  |          **********
       *  |          *     .
       *  |        .........
       *  |        . *
       *  |   ********
       *  | ..*.....
       *  +----------------------->
       *           ^ ^
       *         ix1 ix2
       */

      while ((ix1 != end_ix1) || (ix2 != end_ix2)) {
        /* on each iteration,  compare sample distributions
         * at smallest of (ix1->first, ix2->first)
         */

        bool advance_ix1_flag = false;
        bool advance_ix2_flag = false;

        if (ix1 == end_ix1) {
          /* only ix2 dereferenceable */
          advance_ix2_flag = true;
        } else if (ix2 == end_ix2) {
          /* only ix1 dereferenceable */
          advance_ix1_flag = true;
        } else {
          /* ix1,ix2 both dereferenceable */

	  advance_ix1_flag = (ix1->first <= ix2->first);
	  advance_ix2_flag = (ix2->first <= ix1->first);
        }

        if (advance_ix1_flag) {
          xj1_sum += ix1->first;
          ++ix1;
        }
        if (advance_ix2_flag) {
          xj2_sum += ix2->first;
          ++ix2;
        }

        double p1 = xj1_sum * nr1;
        double p2 = xj2_sum * nr2;

        double dp = std::abs(p1 - p2);

        ks_stat = std::max(ks_stat, dp);
      }

      /* ne = effective #of points to use when comparing 2 sample dist/s */
      double ne = (n1 * n2) / static_cast<double>(n1 + n2);

      return std::pair<double, double>(ne, ks_stat);

#ifdef OBSOLETE
      /* NOTE: this will cost O(n.log(n))
       *       (for empirical distributions with n points)
       *       if we loop over both sets of points in parallel,
       *       can get O(n) solution
       *       
       */

      /* loop over points in *this */
      double ks1 = this->ks_stat_1sided(d2);
      /* loop over points in d2 */
      double ks2 = d2.ks_stat_1sided(*this);

      return std::max(ks1, ks2);
#endif
    } /*ks_stat_2sided*/

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
