/* @file RealizationTracer.hpp */

#pragma once

#include "process/StochasticProcess.hpp"

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
  using Process = xo::process::StochasticProcess<T>;
  /* something like std::pair<utc_nanos, T> */
  using event_type = typename Process::event_type;
  using utc_nanos = xo::time::utc_nanos;
  using nanos = xo::time::nanos;

public:
  RealizationTracer(StochasticProcess<T> * p)
    : current_(event_type(p->t0(), p->t0_value())), process_(p) {}

  event_type const & current_ev() const { return current_; }
  utc_nanos current_tm() const { return current_.first; }
  /* value of this path at time t */
  T const & current_value() const { return current_.second; }

  /* sample with fixed time:
   * - advance to time t+dt,  where t=.current_tm()
   * - return new time and process value
   *
   * can use .advance_dt(dt) to avoid copying T
   */
  std::pair<utc_nanos, T> next_dt(nanos dt) {
    this->advance_dt(dt);

    return this->current_;
  } /*next_dt*/

  std::pair<utc_nanos, T> next_eps(double eps) {
    this->advance_eps(eps);

    return this->current_;
  } /*next_eps*/

  /* sample with fixed time:
   * - advance to point t+dt,  with dt specified.
   */
  void advance_dt(nanos dt) {
    utc_nanos t1 = this->current_.first + dt;

    this->advance_until(t1);
  } /*advance_dt*/

  void advance_until(utc_nanos t1) {
    this->current_.first = t1;
    this->current_.second
      = this->process_->exterior_sample(t1,
					this->current_);
  } /*advance_until*/

#ifdef NOT_IN_USE // need StochasticProcess.hitting_time() for this
  /* sample with max change in process value eps.
   * requires that T defines a norm under which eps
   * can be interpreted
   */
  virtual void advance_eps(double eps) = 0;
#endif

private:
  /* current (time, processvalue) associated with this realization */
  event_type current_;

  /* develop a sampled realization of this stochastic process */
  StochasticProcess<T> * process_ = nullptr;
}; /*RealizationTracer*/

} /*namespace process*/
} /*namespace xo*/

/* end RealizationTracer.hpp */
