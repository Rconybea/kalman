/* @file Empirical.hpp */

#pragma once

#include <map>
#include <cstdint>

namespace xo {
namespace distribution {

  class Counter {
  public:
    Counter() = default;

    uint32_t count() const { return count_; }

    void incr() { ++count_; }

    Counter & operator+=(uint32_t n) { count_ += n; return *this; }
    
  private:
    uint32_t count_ = 0;
  }; /*Counter*/

  /* an empirical distribution,
   * obtained by sorting equally-weighted samples
   */
  template<typename T>
  class Empirical {
  public:
    using SampleMap = std::map<T, Counter>;
    using const_iterator = typename SampleMap::const_iterator;

  public:
    Empirical() = default;

    uint32_t n_sample() const { return n_sample_; }
    const_iterator begin() const { return sample_map_.begin(); }
    const_iterator end() const { return sample_map_.end(); }

    /* introduce one new sample into this distribution */
    void include_sample(T const & x) {
      ++(this->n_sample_);

      Counter & n = this->sample_map_[x];

      n += 1;
    } /*include_sample*/

  private:
    /* count #of calls to .include_sample() */
    uint32_t n_sample_ = 0;

    /* .sample_map_[x] counts the #of times
     * .include_sample(x) has been called.
     */
    SampleMap sample_map_;
  }; /*Empirical*/

} /*namespace distribution*/
} /*namespace xo*/

/* end Empirical.hpp */
