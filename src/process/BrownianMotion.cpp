/* @file BrownianMotion.cpp */

#include "time/Time.hpp"
#include "BrownianMotion.hpp"
#include <cmath>

namespace xo {
  using xo::time::utc_nanos;
  using logutil::scope;
  using logutil::xtag;

  namespace process {
    double
    BrownianMotionBase::variance_dt(nanos dt) const
    {
      constexpr uint64_t c_sec_per_day = (24L * 3600L);
      constexpr double c_day_per_sec = (1.0 / c_sec_per_day);

      /* time-to-horizon in nanos */
      double dt_sec = std::chrono::duration<double>(dt).count();
      double dt_day = dt_sec * c_day_per_sec;

      return this->vol2_day_ * dt_day;
    } /*variance_dt*/

    double
    BrownianMotionBase::exterior_sample_impl(utc_nanos t,
					     BrownianMotionBase::event_type const & lo,
					     double x0)
    {
      constexpr bool c_logging_enabled = false;
      scope lscope("BrownianMotionBase::exterior_sample_impl", c_logging_enabled);

      /* sample brownian motion starting at t0;
       * offset by lo.second
       */

      utc_nanos lo_tm = lo.first;
      double lo_x = lo.second;
      
      nanos dt = (t - lo_tm);

      /* variance at horizon t,  relative to value at lo.first */
      double var = this->variance_dt(dt);

      /* scale for variance of B(t) - B(lo) */
      double dx = ::sqrt(var) * x0;

      double sample = lo_x + dx;

      if(c_logging_enabled) {
	lscope.log("result",
		   xtag("start-time", this->t0()),
		   xtag("vol2-day", this->vol2_day()),
		   xtag("lo.tm", lo_tm),
		   xtag("lo.x", lo_x),
		   xtag("dt-us", std::chrono::duration_cast<std::chrono::microseconds>(dt).count()),
		   xtag("var", var),
		   xtag("dx", dx));
      }

      return sample;
    } /*exterior_sample_impl*/

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
