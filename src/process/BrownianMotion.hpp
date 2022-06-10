/* @file BrownianMotion.hpp */

#pragma once

#include "process/StochasticProcess.hpp"
#include "random/Normal.hpp"
#include <chrono>

namespace xo {
namespace process {
  // representation for brownian motion.
  //
  // starting value of zero at time t0.
  // for process with volatility s,  variance for horizon dt is
  //   V = (s^2).dt
  //
  // ofc this means volatility has units 1/sqrt(t)
  //
  class BrownianMotion : public StochasticProcess<double> {
  public:
    using NormalGen = xo::random::NormalGen<std::mt19937>;
    using nanos = xo::time::nanos;
    
  public:
    BrownianMotion(utc_nanos t0,
		   double sdev,
		   uint64_t seed)
      : t0_{t0},
	volatility_{sdev},
	vol2_day_{(sdev * sdev) * (1.0 / 365.25)},
	rng_{NormalGen::make(seed, 0.0 /*mean*/, 1.0 /*sdev*/)}
    {}
    virtual ~BrownianMotion() = default;

    /* t0.  start time,
     * sdev.  annual sqrt volatility
     * seed.  initialize pseudorandom-number generator
     */
    static BrownianMotion * make(utc_nanos t0,
				 double sdev,
				 uint64_t seed) {
      return new BrownianMotion(t0, sdev, seed);
    } /*make*/

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
				       event_type const & lo) override;
    /* sample this process at time t,
     * given glb sample for this process lo={t_lo, x_lo},
     * and lub sample for this process of hi={t_hi, x_hi},
     * t_lo<t<t_hi
     */
    virtual value_type interior_sample(utc_nanos t,
				       event_type const & lo,
				       event_type const & hi) override;

#ifdef NOT_IN_USE
    /* sample hitting time
     *   T(a) = inf{t : P(t)=a, t>t1} for process hitting value a,
     * given preceding known value
     *   {t1, v1} = {lo.first, lo.second}
     */
    virtual utc_nanos hitting_time(double const & a,
				   event_type const & lo) override;
#endif

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
    NormalGen::generator_type rng_;
  }; /*BrownianMotion*/
} /*namespace process*/
} /*namespace xo*/

/* end BrownianMotion.hpp */
