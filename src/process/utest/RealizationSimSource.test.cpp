/* @file RealizationSimSource.test.cpp */

#include "process/RealizationSimSource.hpp"
#include "process/LogNormalProcess.hpp"
#include "process/BrownianMotion.hpp"
#include "random/xoshiro256.hpp"
#include "simulator/Simulator.hpp"
#include "logutil/printer.hpp"
#include "logutil/scope.hpp"
#include "catch2/catch.hpp"

namespace xo {
  using xo::sim::Simulator;
  using xo::process::RealizationSimSource;
  using xo::process::RealizationTracer;
  using xo::process::LogNormalProcess;
  using xo::process::ExpProcess;
  using xo::process::BrownianMotion;
  using xo::random::xoshiro256ss;
  using xo::refcnt::rp;
  using xo::time::Time;
  using xo::time::seconds;
  using xo::time::utc_nanos;
  //using xo::print::printer;
  using logutil::scope;
  using logutil::xtag;
  using std::chrono::hours;
  using std::chrono::minutes;

  namespace ut {
    /* TODO: move this to time/utest/ */
    TEST_CASE("time-formatting", "[time][print]") {
      /* TODO: unit test for time conversions */

      constexpr char const * c_self = "TEST_CASE:time-formatting";
      constexpr bool c_logging_enabled = true;

      utc_nanos t0 = Time::ymd_hms_usec(20220610 /*ymd*/,
					162905 /*hms*/,
					123456 /*usec*/);

      std::stringstream ss;
      xo::time::Time::print_ymd_hms_usec(t0, ss);

      REQUIRE(ss.str() == "20220610:162905.123456");

#ifdef NOT_IN_USE
      BrownianMotion bm = BrownianMotion::make(xxx t0,
					       xxx dev,
					       xxx seed);
#endif
    } /*TEST_CASE(time-formatting)*/

    /* TODO: move this to simulator/utest/ */
    TEST_CASE("empty-simulation", "[simulation][trivial]") {
      constexpr char const * c_self = "TEST_CASE:empty-simulation";
      constexpr bool c_logging_enabled = true;

      /* arbitrary 'starting time' */
      utc_nanos t0 = Time::ymd_hms_usec(20220610 /*ymd*/,
					162905 /*hms*/,
					123456 /*usec*/);

      Simulator sim(t0);

      REQUIRE(sim.is_exhausted());

      utc_nanos t1 = t0 + hours(1);

      sim.run_until(t1);

      REQUIRE((sim.is_exhausted() || (sim.next_tm() > t1)));
    } /*TEST_CASE(empty-simulation)*/

    /* test simulator with a single source */
    TEST_CASE("sim-brownian-motion", "[process][simulation]") {
      constexpr char const * c_self = "TEST_CASE:sim-brownian-motion";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

      /* arbitrary 'starting time' */
      utc_nanos t0 = Time::ymd_hms_usec(20220610 /*ymd*/,
					162905 /*hms*/,
					123456 /*usec*/);

      Simulator sim(t0);

      REQUIRE(sim.is_exhausted());

      std::unique_ptr<BrownianMotion<xoshiro256ss>> bm
	= BrownianMotion<xoshiro256ss>::make(t0,
					     0.30 /*sdev -- annualized volatility*/,
					     12345678UL /*seed*/);
      RealizationTracer<double> tracer(bm.get());

      std::vector<std::pair<utc_nanos,double>> sample_v;

      auto sink
	= ([&sample_v]
	   (std::pair<utc_nanos,double> const & ev)
	{ sample_v.push_back(ev); });

      /* what is step dt? */
      rp<RealizationSimSource<double, decltype(sink)>>
	sim_source
	= RealizationSimSource<double, decltype(sink)>::make(&tracer,
							     std::chrono::seconds(1) /*ev_interval_dt*/,
							     sink);

      sim.add_source(sim_source);

      utc_nanos t1 = t0 + minutes(1);

      sim.run_until(t1);

      /* 1-minute simulation with 1-second samples */
      REQUIRE(sample_v.size() == 61);

      utc_nanos sample_t0 = sample_v[0].first;

      for(size_t i = 0; i < sample_v.size(); ++i) {
	REQUIRE(sample_v[i].first == t0 + seconds(i));
      }

      //lscope.log(xtag("sample_v.size", sample_v.size()));

    } /*TEST_CASE("sim-brownian-motion")*/

    TEST_CASE("sim-lognormal", "[process][simulation]") {
      constexpr char const * c_self = "TEST_CASE:sim-brownian-motion";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

      /* arbitrary 'starting time' */
      utc_nanos t0 = Time::ymd_hms_usec(20220610 /*ymd*/,
					162905 /*hms*/,
					123456 /*usec*/);

      Simulator sim(t0);

      REQUIRE(sim.is_exhausted());

      std::unique_ptr<ExpProcess> ebm
	(LogNormalProcess::make<xoshiro256ss, uint64_t>
	   (t0,
	    0.30 /*sdev -- annualized volatility*/,
	    12345678UL /*seed*/));

      /* recover the exponentiated process,  for testing */
      //StochasticProcess<double> * bm = ebm->exponent_process();

      RealizationTracer<double> tracer(ebm.get());

      /* will be: samples from log-normal brownian motion */
      std::vector<std::pair<utc_nanos,double>> sample_v;

      /* collect process samples as sim runs */
      auto sink
	= ([&sample_v]
	   (std::pair<utc_nanos,double> const & ev)
	{ sample_v.push_back(ev); });

      rp<RealizationSimSource<double, decltype(sink)>>
	sim_source
	= RealizationSimSource<double, decltype(sink)>::make(&tracer,
							     std::chrono::seconds(1) /*ev_interval_dt*/,
							     sink);

      sim.add_source(sim_source);

      utc_nanos t1 = t0 + minutes(1);

      sim.run_until(t1);

      /* 1-minute simulation with 1-second samples */
      REQUIRE(sample_v.size() == 61);

      utc_nanos sample_t0 = sample_v[0].first;

      for(size_t i = 0; i < sample_v.size(); ++i) {
	REQUIRE(sample_v[i].first == t0 + seconds(i));
	/* exponentiated process will have strictly +ve values */
	REQUIRE(sample_v[i].second > 0.0);
      }

      if(c_logging_enabled)
	lscope.log(xtag("sample_v.size", sample_v.size()));
    } /*TEST_CASE("sim-lognormal")*/
  } /*namespace ut*/
} /*namespace xo*/

/* end RealizationSimSource.test.cpp */
