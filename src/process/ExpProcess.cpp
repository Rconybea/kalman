/* @file ExpProcess.cpp */

#include "time/Time.hpp"
#include "ExpProcess.hpp"

namespace xo {
  using logutil::scope;
  using logutil::xtag;

  namespace process {
    /* note: lo is a sample from the exponentiated process;
     *       must take log to get sample from the exponent process
     */
    ExpProcess::value_type
    ExpProcess::exterior_sample(utc_nanos t,
				event_type const & lo)
    {
      constexpr bool c_logging_enabled = false;
      scope lscope("ExpProcess::exterior_sample", c_logging_enabled);

      double lo_value = lo.second;
      double log_lo_value = ::log(lo.second / this->scale_);

      double e
	= (this->exponent_process_->exterior_sample
	   (t,
	    event_type(lo.first, log_lo_value)));

      double retval = this->scale_ * ::exp(e);

      if(c_logging_enabled) {
	lscope.log("result",
		   xtag("t", t),
		   xtag("lo.tm", lo.first),
		   xtag("lo.value", lo_value),
		   xtag("log(lo.value/m)", log_lo_value),
		   xtag("m", this->scale_),
		   xtag("e", e),
		   xtag("retval", retval));
      }

      return retval;
    } /*exterior_sample*/

  } /*namespace process*/
} /*namespace xo*/

/* end ExpProcess.cpp */
