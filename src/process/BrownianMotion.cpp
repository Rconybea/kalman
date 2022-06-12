/* @file BrownianMotion.cpp */

#include "BrownianMotion.hpp"
#include <cmath>

namespace xo {
  using xo::time::utc_nanos;

  namespace process {
#ifdef NOT_IN_USE
    utc_nanos
    BrownianMotion::hitting_time(double const & a,
				 event_type const & lo)
    {
      /* (1)
       * probability density function p1(s)
       * giving hitting time for brownian motion starting at 0,
       * first time to reach a constant barrier a:
       * 
       *                                a^2
       *                              - ---
       *                  a             2.s
       *    p1(s) = ------------- . e
       *            sqrt(2.pi.s^3)
       *
       * (2)
       * we also know probability density function p2(s)
       * giving hitting time for brownian motion starting at 0,
       * first time to reach expanding barrier a + ct:
       * (i.e. T2 = inf{t : B(t) = c.t + a, t > 0})
       *
       *                                (c.s + a)^2
       *                              - -----------
       *                  a                 2.s
       *    p2(s) = -------------- . e 
       *            sqrt(2.pi.s^3)
       *    
       */
    } /*hitting_time*/
#endif
  } /*namespace process*/
} /*namespace xo*/

/* end BrownianMotion.cpp */
