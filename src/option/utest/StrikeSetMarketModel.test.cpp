/* @file StrikeSetMarketModel.cpp */

#include "option/StrikeSetMarketModel.hpp"
#include "process/LogNormalProcess.hpp"
#include "simulator/Simulator.hpp"
#include "random/xoshiro256.hpp"
#include "time/Time.hpp"
#include <catch2/catch.hpp>

namespace xo {
  using xo::option::StrikeSetMarketModel;
  using xo::option::OptionStrikeSet;
  using xo::process::LogNormalProcess;
  using xo::process::RealizationTracer;
  using xo::sim::Simulator;
  using xo::random::xoshiro256ss;
  using xo::time::Time;
  using xo::time::utc_nanos;
  using std::chrono::minutes;

  namespace ut {
    TEST_CASE("strikeset-market-model-empty", "[marketmodel]") {
      auto optionset
	= OptionStrikeSet::empty();

      REQUIRE(optionset.get() != nullptr);

      utc_nanos t0 = Time::ymd_hms_usec(20220617 /*ymd*/,
					173905 /*hms*/,
					123456 /*usec*/);
      double sdev = 2.5; /*250% vol !*/
      uint64_t seed = 14950319842636922572UL;
      //uint64_t seed = 14950349842636922572UL;

      auto process
	= LogNormalProcess::make<xoshiro256ss, uint64_t>(t0, 1000.0 /*scale*/, sdev, seed);

      REQUIRE(process.get() != nullptr);

      auto tracer
	= RealizationTracer<double>::make(process.get());

      REQUIRE(tracer.get() != nullptr);

      auto model
	= StrikeSetMarketModel::make(optionset,
				     tracer,
				     std::chrono::seconds(1) /*ul_ev_interval_dt*/);

      REQUIRE(model.get() != nullptr);

      auto simulator
	= Simulator::make(t0);

      REQUIRE(simulator.get() != nullptr);

      model->bind_reactor(simulator.borrow());

      /* run simulation */

      utc_nanos t1 = t0 + minutes(1);

      simulator->run_until(t1);
    } /*TEST_CASE(strikeset-market-model-empty)*/

    TEST_CASE("strikeset-market-model-one", "[marketmodel]")
    {
      utc_nanos t0_sod = Time::ymd_hms_usec(20220617 /*ymd*/,
					    93000 /*hms*/,
					    0 /*usec*/);
      utc_nanos t0 = Time::ymd_hms_usec(20220617 /*ymd*/,
					173905 /*hms*/,
					123456 /*usec*/);

      utc_nanos expiry1_tm = t0_sod + std::chrono::days(7);

      auto optionset
	= OptionStrikeSet::regular(1 /*n*/,
				   1000.0 /*lo_strike*/,
				   5.0 /*d_strike*/,
				   expiry1_tm);

      REQUIRE(optionset.get() != nullptr);

      double sdev = 2.5; /*250% vol !*/
      uint64_t seed = 14950319842636922572UL;
      //uint64_t seed = 14950349842636922572UL;

      auto process
	= LogNormalProcess::make<xoshiro256ss, uint64_t>(t0, 1000.0 /*scale*/, sdev, seed);

      REQUIRE(process.get() != nullptr);

      auto tracer
	= RealizationTracer<double>::make(process.get());

      REQUIRE(tracer.get() != nullptr);

      auto model
	= StrikeSetMarketModel::make(optionset,
				     tracer,
				     std::chrono::seconds(1) /*ul_ev_interval_dt*/);

      REQUIRE(model.get() != nullptr);

      auto simulator
	= Simulator::make(t0);

      REQUIRE(simulator.get() != nullptr);

      model->bind_reactor(simulator.borrow());

      /* run simulation */

      utc_nanos t1 = t0 + minutes(1);

      simulator->run_until(t1);
    } /*TEST_CASE(strikeset-market-model-empty)*/

  } /*namespace ut*/

} /*namespace xo*/

/* end StrikeSetMarketModel.cpp */
