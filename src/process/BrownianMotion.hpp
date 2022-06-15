/* @file BrownianMotion.hpp */

#pragma once

#include "process/StochasticProcess.hpp"
#include "random/Normal.hpp"
#include <memory>
#include <chrono>

namespace xo {
  namespace process {
    /* representation for brownian motion.
     *
     * starting value of zero at time t0.
     * for process with volatility s,  variance for horizon dt is
     *   V = (s^2).dt
     *
     * ofc this means volatility has units 1/sqrt(t)
     *
     * event_type:  something like std::pair<utc_nanos, double>
     * value_type:  double
     */
    template<class RngEngine>
    class BrownianMotion : public StochasticProcess<double> {
    public:
      using NormalGen = xo::random::NormalGen<RngEngine>;
      using nanos = xo::time::nanos;

    public:
      /* t0.  start time,
       * sdev.  annual sqrt volatility
       * seed.  initialize pseudorandom-number generator
       */
      template<class Seed>
      static ref::rp<BrownianMotion<RngEngine>> make(utc_nanos t0,
						     double sdev,
						     Seed const & seed)
      {
	return new BrownianMotion<RngEngine>(t0, sdev, seed);
      } /*make*/

      virtual ~BrownianMotion() = default;

      /* brownian motion with constant volatility at this level */
      double volatility() const { return volatility_; }

      /* compute variance that accumulates over time interval dt
       * for this brownian motion
       */
      double variance_dt(nanos dt) const;

      // ----- inherited from StochasticProcess<> -----

      virtual utc_nanos t0() const override { return t0_; }
      virtual double t0_value() const override { return 0.0; }

      /* sample this process at time t,
       * given glb sample for this process lo={t_lo, x_lo}, t>t_lo
       */
      virtual value_type exterior_sample(utc_nanos t,
                                         event_type const &lo) override;
      /* sample this process at time t,
       * given glb sample for this process lo={t_lo, x_lo},
       * and lub sample for this process of hi={t_hi, x_hi},
       * t_lo<t<t_hi
       */
      virtual value_type interior_sample(utc_nanos t, event_type const &lo,
                                         event_type const &hi) override;

#ifdef NOT_IN_USE
      /* sample hitting time
       *   T(a) = inf{t : P(t)=a, t>t1} for process hitting value a,
       * given preceding known value
       *   {t1, v1} = {lo.first, lo.second}
       */
      virtual utc_nanos hitting_time(double const &a,
                                     event_type const &lo) override;
#endif

    private:
      template<class Seed>
      BrownianMotion(utc_nanos t0, double sdev, Seed const & seed)
          : t0_{t0}, volatility_{sdev}, vol2_day_{(sdev * sdev) *
                                                  (1.0 / 365.25)},
            rng_{NormalGen::make(seed, 0.0 /*mean*/, 1.0 /*sdev*/)} {}

    private:
      /* starting time for this process */
      utc_nanos t0_;

      /* annual volatility (1-year := 365.25 days) for this process */
      double volatility_ = 0.0;

      /* daily variance for this brownian motion */
      double vol2_day_ = 0.0;

      /* generates normally-distributed pseudorandom numbers,
       * distributed according to N(0,1)
       */
      typename NormalGen::generator_type rng_;
    }; /*BrownianMotion*/

    template<typename RngEngine>
    double
    BrownianMotion<RngEngine>::variance_dt(nanos dt) const
    {
      constexpr uint64_t c_sec_per_day = (24L * 3600L);
      constexpr double c_day_per_sec = (1.0 / c_sec_per_day);

      /* time-to-horizon in nanos */
      double dt_sec = std::chrono::duration<double>(dt).count();
      double dt_day = dt_sec * c_day_per_sec;

      return this->vol2_day_ * dt_day;
    } /*variance_dt*/

    template<typename RngEngine>
    double
    BrownianMotion<RngEngine>::interior_sample(utc_nanos ts,
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

    template<typename RngEngine>
    double
    BrownianMotion<RngEngine>::exterior_sample(utc_nanos t,
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


  }  /*namespace process*/
}  /*namespace xo*/

/* end BrownianMotion.hpp */
