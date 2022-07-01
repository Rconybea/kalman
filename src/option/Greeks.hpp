/* @file Greeks.hpp */

#pragma once

#include "time/Time.hpp"
#include "option/OptionId.hpp"

namespace xo {
  namespace option {
    /* encapsulate result of greeks calculation for an option */
    class Greeks {
    public:
      Greeks() = default;
      Greeks(double tv, double delta, double gamma, double vega, double theta, double rho)
	: tv_{tv}, delta_{delta}, gamma_{gamma}, vega_{vega}, theta_{theta}, rho_{rho} {}

      double tv() const { return tv_; }
      double delta() const { return delta_; }
      double gamma() const { return gamma_; }
      double vega() const { return vega_; }
      double theta() const { return theta_; }
      double rho() const { return rho_; }

    private:
      /* theory price, in per-share units */
      double tv_ = 0.0;
      /* option delta;  dimensionless (always in [-1, +1]) */
      double delta_ = 0.0;
      /* option gamma;  per 100% change in spot */
      double gamma_ = 0.0;
      /* option vega;  per 100% change in volatility */
      double vega_ = 0.0;
      /* option theta;  change in value w.r.t elapsed time */
      double theta_ = 0.0;
      /* option rho;  change in value w.r.t interest rates */
      double rho_ = 0.0;
    }; /*Greeks*/

    /* encapsulate option Greeks as a simulator event.
     * this adds:
     * - total ordering of greeks events by timestamp
     * - remember option id for each event
     * - remember timestamp for each event + report via .tm() method
     */
    class GreeksEvent : public Greeks {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      GreeksEvent(utc_nanos tm, OptionId oid,
		  double tv, double delta, double gamma, double vega, double theta, double rho)
	: Greeks(tv, delta, gamma, vega, theta, rho),
	  tm_(tm), oid_(oid) {}

      /* compare events by (.tm(), .oid()) */
      static int64_t compare(GreeksEvent const & x,
			     GreeksEvent const & y);

      utc_nanos tm() const { return tm_; }
      OptionId oid() const { return oid_; }

    private:
      /* event timestamp -- asof time for these greeks */
      utc_nanos tm_;
      /* identifies option assoc'd with this greeks event;
       * use to preserve identity when multiplexing events for different vanilla option
       * instances
       */
      OptionId oid_;
    }; /*GreeksEvent*/

    /* consider greeks _events_ equal if they have the same instrument + timestamp,
     * event if greeks values {delta, gamma, ...} are not the same 
     */
    inline bool operator==(GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) == 0; }
    inline bool operator!=(GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) != 0; }
    inline bool operator< (GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) <  0; }
    inline bool operator<=(GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) <= 0; }
    inline bool operator> (GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) >  0; }
    inline bool operator>=(GreeksEvent const & x, GreeksEvent const & y) { return GreeksEvent::compare(x, y) >= 0; }
      
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.hpp */
