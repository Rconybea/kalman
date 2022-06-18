/* LogNormalProcess.hpp */

#pragma once

#include "process/BrownianMotion.hpp"
#include "process/ExpProcess.hpp"

namespace xo {
  namespace process {

    /* log-normal process -- i.e. logs follow brownian motion
     */
    class LogNormalProcess {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      /* log-normal process starting at (t0, x0) */
      template<typename RngEngine, typename Seed>
      static ref::rp<ExpProcess> make(utc_nanos t0, double x0,
				      double sdev, Seed const & seed) {

	ref::rp<BrownianMotion<RngEngine>> bm
	  = BrownianMotion<RngEngine>::make(t0, sdev, seed);

        return ExpProcess::make(x0 /*scale*/, bm);
      } /*make*/
    }; /*LogNormalProcess*/

  } /*namespace process*/
} /*namespace xo*/

/* end LogNormalProcess.hpp */
