/* @file RealizationTracer.hpp */

#pragma once

#include "time/Time.hpp"
#include <utility>

namespace xo {
namespace process {

// One-way iteration over a realization (i.e. sampled path)
// belonging to a stochastic process.
// has a monotonically increasing 'current time'.
// can be adapted as a simulation source
//
template <typename T>
class RealizationTracer {
public:
  using utc_nanos = xo::time::utc_nanos;
  using nanos = xo::time::nanos;

public:
  /* current time t associated with this tracer */
  utc_nanos current_tm() const;
  /* value of this path at time t */
  T const & value() const;

  /* sample with fixed time:
   * - advance to time t+dt,  where t=.current_tm()
   * - return new time and process value
   *
   * can use .advance_dt(dt) to avoid copying T
   */
  std::pair<utc_nanos, T> next_dt(nanos dt) {
    this->advance_dt(dt);

    return std::make_pair(this->current_tm(),
			  this->value());
  } /*next_dt*/

  std::pair<utc_nanos, T> next_eps(double eps) {
    this->advance_eps(eps);

    return std::make_pair(this->current_tm(),
			  this->value());
  } /*next_eps*/

  /* sample with fixed time:
   * - advance to point t+dt,  with dt specified.
   */
  virtual void advance_dt(nanos dt) = 0;

  /* sample with max change in process value eps.
   * requires that T defines a norm under which eps
   * can be interpreted
   */
  virtual void advance_eps(double eps) = 0;

}; /*RealizationTracer*/

} /*namespace process*/
} /*namespace xo*/

/* end RealizationTracer.hpp */
