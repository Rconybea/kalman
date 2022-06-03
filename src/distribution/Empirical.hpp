/* @file Empirical.hpp */

#pragma once

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

  /* an empirical distribution,
   * obtained by sorting equally-weighted samples
   */
  template<typename T>
  class Empirical {
  public:
    using SampleMap = xo::tree::RedBlackTree<T,
					     Counter,
					     xo::tree::SumReduce<uint32_t>>;
    using const_iterator = typename SampleMap::const_iterator;

  public:
    Empirical() = default;

    uint32_t n_sample() const { return n_sample_; }
    const_iterator begin() const { return sample_map_.begin(); }
    const_iterator end() const { return sample_map_.end(); }

    /* introduce one new sample into this distribution */
    void include_sample(T const & x) {
      ++(this->n_sample_);

      /* note: xo::tree::RedBlackTree doesn't provide the usual reference result
       *       from operator[];  it needs to intervene after assignment to update
       *       order statistics
       */
      auto lhs = this->sample_map_[x];

      lhs += 1;
    } /*include_sample*/

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
