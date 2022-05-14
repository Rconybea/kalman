/* @file Realization.hpp */

#pragma once

#include "StochasticProcess.hpp"
#include "time/Time.hpp"
#include <boost/range.hpp>
#include <map>

namespace xo {
namespace process {

// realization of a stochastic process.
// interface designed to allow for lazy evaluation.
//
// since a process connects a family of random variables,
// a single process can have a generally unbounded number of distinct realizations.
//
// implications:
// - can only realize (or observe) a finite set of instants.
// - given process evolves continuously,
//   want ability to revisit intervals that may already contain some realized instants.
// - achieve this by allowing for caching behavior
//
template<typename T>
class Realization {
public:
  using utc_nanos = xo::time::utc_nanos;
  using KnownMap = std::map<utc_nanos, T>;
  using KnownIterator = typename KnownMap::const_iterator;
  using KnownRange = boost::iterator_range<KnownIterator>;

public:
  static Realization * make(StochasticProcess<T> * p) { return new Realization(p); }

private:
  explicit Realization(StochasticProcess<T> * p) : process_{p} {}

  StochasticProcess<T> * process() const { return process_; }

  utc_nanos t0() const { return process_->t0(); }

  size_t n_known() const { return this->known_map_.size(); }

  /* require: .n_known() > 0 */
  utc_nanos lo_tm() const { return this->known_map_.begin().first(); }
  utc_nanos hi_tm() const { return this->known_map_.rbegin().first(); }

  KnownRange known_range() const { return boost::make_iterator_range(this->known_map_); }

  // concept:
  //   realized_range() -> iterator_range<IT>

private:
  /* stochastic process from which this realization is sampled */
  StochasticProcess<T> * process_ = nullptr;

  /* process value (for this realization) has been established (sampled)
   * at each time t in {.known_map[].first}
   */
  KnownMap known_map_;
}; /*Realization*/

} /*namespace process*/
} /*namespace xo*/

/* end Realization.hpp */
