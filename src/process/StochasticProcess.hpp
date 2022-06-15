/* @file StochasticProcess.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include "time/Time.hpp"
#include <utility>

namespace xo {
namespace process {

// abstraction for a stochastic process.
// - represents a probability space:
//   - a collection of paths
//   - an associated probability measure on path sapce
// - paths may vary continuously with time
// - need not be continuous
// - want to be able to use in simulation,
//   in which case will likely require some discretization
//
template<typename T>
class StochasticProcess : public refcnt::Refcount {
public:
  using value_type = T;
  using utc_nanos = xo::time::utc_nanos;
  using event_type = std::pair<utc_nanos, T>;

public:
  virtual ~StochasticProcess() = default;

  /* starting time for this process */
  virtual utc_nanos t0() const = 0;

  /* starting value of this process */
  virtual T t0_value() const = 0;

  /* sample this process at time t,
   * given preceding known value
   *   {t1, v1}
   * with t1 < t
   */
  virtual value_type exterior_sample(utc_nanos t,
				     event_type const & lo) = 0;

  /* sample this process at time t,
   * given surrounding known values
   *   {t1, v1}, {t2, v2}
   * with t1 < t < t2
   */
  virtual value_type interior_sample(utc_nanos t,
				     event_type const & lo,
				     event_type const & hi) = 0;

#ifdef NOT_IN_USE
  /* sample hitting time
   *    T(a) = inf{t : P(t)=a, t>t1} for process hitting value a,
   * given preceding known value
   *   {t1, v1} = {lo.first, lo.second}
   */
  virtual utc_nanos hitting_time(T const & a,
				 event_type const & lo) = 0;
#endif				 

}; /*StochasticProcess*/

} /*namespace process*/
} /*namespace xo*/

/* end StochasticProcess.hpp */
