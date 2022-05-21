/* @file ExpProcess.cpp */

#include "ExpProcess.hpp"

namespace xo {
namespace process {

value_type
ExpProcess::exterior_sample(utc_nanos t,
                                       event_type const &lo) override {
  double e = this->exponent_process_->exterior_sample(
      t, event_type(lo.first, ::log(lo.second)));

  return ::exp(e);
} /*exterior_sample*/

value_type
ExpProcess::interior_sample(utc_nanos t,
			    event_type const & lo,
			    event_type const & hi) override {
  double e
    = (this
       ->exponent_process_
       ->interior_sample(t,
			 event_type(lo.first,
				    ::log(lo.second)),
			 event_type(hi.first,
				    ::log(hi.second))));

  return ::exp(e);
} /*interior_sample*/

} /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.cpp */
