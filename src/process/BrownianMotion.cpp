/* @file BrownianMotion.cpp */

#include "BrownianMotion.hpp"
#include <cmath>

namespace xo {
  using xo::time::utc_nanos;

  namespace process {
    double
    BrownianMotion::variance_dt(nanos dt) const
    {
      constexpr uint64_t c_sec_per_day = (24L * 3600L);
      constexpr double c_day_per_sec = (1.0 / c_sec_per_day);

      /* time-to-horizon in nanos */
      double dt_sec = std::chrono::duration<double>(dt).count();
      double dt_day = dt_sec * c_day_per_sec;

      return this->vol2_day_ * dt_day;
    } /*variance_dt*/

    double
    BrownianMotion::interior_sample(utc_nanos ts,
				    event_type const & lo,
				    event_type const & hi)
    {
      /* suppose we know values of a brownian motion
       * at two points t1, t2:
       *   x1 = B(t1)
       *   x2 = B(t2)
       *
       * Want to sample B for some particular time ts in (t1,t2).
       *
       * First step is to de-drift B:
       * 
       * considered sheared process B'(t):
       *  B'(dt) = -B(t1) + B(t1+dt) - u.dt,
       *             where u = (x2-x1).(t2-t1),   t in (t1,t2)
       * then
       *  B'(0)     = 0
       *  E[B'(dt)] = 0
       *  V[B'(dt)] ~ dt
       *
       * so B' is also a brownian motion.
       * We want to sample the conditional process 
       *   B''(dt) = {B'(dt) | B'(t2-t1)=0} at some point ts in (0, t2-t1).
       * The condition B'(t2-t1)=0 gives us:
       *   B(t1) + B''(t-t1) = B(t1),  t=t1
       *   B(t1) + B''(t-t1) = B(t2),  t=t2
       * 
       * At ts:
       * - the increment x = B(ts) - B(t1) is normally distributed,
       *   with variance proportional to (ts - t1).
       * - the increment y = B(t2) - B(ts) is normally distributed,
       *   with variance proportional to (t2 - ts).
       *
       * Using bivariate normal prob density of two vars x,y
       * with sdev sx, sy:
       *
       *                 1           /          x^2      y^2    \
       *   p(x,y) = ---------- . exp | -(1/2) (------ + ------) |
       *            2.pi.sx.sy       \          sx^2     sy^2   /
       *           
       * we can condition on y=-x to get conditional probability distribution
       *
       *               1           /         x^2 . sy^2  +  y^2 . sx^2  \
       *   p(x) = ---------- . exp | -(1/2) --------------------------- |
       *          2.pi.sx.sy       \               sx^2 . sy^2          /
       *
       *
       *               1           /         x^2 . sy^2  +  y^2 . sx^2  \
       *        = ---------- . exp | -(1/2) --------------------------- |
       *          2.pi.sx.sy       \               sx^2 . sy^2          /
       *  
       *               1           /            x^2 . (sy^2 + sx^2)     \
       *        = ---------- . exp | -(1/2) --------------------------- |
       *          2.pi.sx.sy       \               sx^2 . sy^2          /
       *  
       *                 /  sx^2 . sy^2  \
       *  let sxy = sqrt | ------------- |
       *                 \  sx^2 + sy^2  /
       *
       * then
       *               1           /         x^2  \
       *   p(x) = ---------- . exp | -(1/2) ----- |
       *          2.pi.sx.sy       \        sxy^2 /
       *  
       * which is density for normal distribution with variance sxy^2,
       * (scaled by constant 1 / sqrt(sx^2 + sy^2));
       *
       * e.g. at midpoint between t1 and t2,  is sx^2 = sy^2 = 1/2 :
       *   sxy^2 = 1/4
       *   
       * which is 1/2 the variance we'd see at midpoint if not constrained
       * to B(t2)=x2
       */   

      utc_nanos lo_tm	= lo.first;
      double lo_x	= lo.second;
      utc_nanos hi_tm	= hi.first;
      double hi_x	= hi.second;

      double t_frac = (ts - lo_tm) / (hi_tm - lo_tm);

      /* compute mean value, at t, relative to B(lo),
       * of all brownian motions on [lo, hi] that
       * start from B(lo) and end at B(hi),
       *
       * i.e. applying drift u = (x2 - x1)/(t2 - t1) two stationary BM
       */
      double mean_dx = (hi_x - lo_x) * t_frac;

      /* t splits the interval [t1,t2] into two subintervals
       * [t1,t] and [t,t2].  compute variances of brownian motion
       * increments [t1,t], [t,t2]:
       */
      double var1 = this->variance_dt(ts - lo_tm);
      double var2 = this->variance_dt(hi_tm - ts);

      /* variance for B(ts) is (var1 * var2 / (var1 + var2)) */
      double vars = var1 * var2 / (var1 + var2);

      /* sample from N(0,1) */
      double xs = this->rng_();

      /* scale for variance of B(ts) */
      double dx = ::sqrt(vars) * xs;

      double x = lo_x + mean_dx + dx;

      return x;
    } /*interior_sample*/

    double
    BrownianMotion::exterior_sample(utc_nanos t,
				    event_type const & lo)
    {
      /* sample brownian motion starting at t0;
       * offset by lo.second
       */

      utc_nanos lo_tm = lo.first;
      double lo_x = lo.second;
      
      nanos dt = (t - lo_tm);

      /* variance at horizon t,  relative to value at lo.first */
      double var = this->variance_dt(dt);

      /* sample from N(0,1) */
      double x0 = this->rng_();

      /* scale for variance of B(t) - B(lo) */
      double dx = ::sqrt(var) * x0;

      double sample = lo_x + dx;

      return sample;
    } /*exterior_sample*/

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
