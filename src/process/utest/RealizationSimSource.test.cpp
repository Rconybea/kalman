/* @file RealizationSimSource.test.cpp */

#include "process/RealizationSimSource.hpp"
#include "simulator/Simulator.hpp"
#include "logutil/printer.hpp"
#include "logutil/scope.hpp"
#include "catch2/catch.hpp"

namespace xo {
  using xo::sim::Simulator;
  //using xo::process::RealizationSimSource;
  using xo::time::Time;
  using xo::time::utc_nanos;
  //using xo::print::printer;
  //using logutil::scope;
  //using logutil::xtag;
  using std::chrono::hours;

  namespace ut {
    /* TODO: move this to time/utest/ */
    TEST_CASE("time-formatting", "[time formatting]") {
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
    TEST_CASE("empty-simulation", "[simulation with zero events]") {
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
  } /*namespace ut*/
} /*namespace xo*/

/* end RealizationSimSource.test.cpp */
