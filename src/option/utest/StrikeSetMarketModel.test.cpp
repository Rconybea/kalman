/* @file StrikeSetMarketModel.cpp */

#include "option/StrikeSetMarketModel.hpp"
#include "option/OmdCallback.hpp"
#include "process/LogNormalProcess.hpp"
#include "simulator/Simulator.hpp"
#include "random/xoshiro256.hpp"
#include "time/Time.hpp"
#include <catch2/catch.hpp>

namespace xo {
  using xo::option::StrikeSetMarketModel;
  using xo::option::OptionStrikeSet;
  using xo::option::PricingContext;
  using xo::option::BboTick;
  using xo::option::Pxtick;
  using xo::option::OptionId;
  using xo::process::LogNormalProcess;
  using xo::process::RealizationTracer;
  using xo::sim::Simulator;
  using xo::random::xoshiro256ss;
  using xo::time::Time;
  using xo::time::utc_nanos;
  using logutil::scope;
  using logutil::xtag;
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

      auto pricing_cx
	= PricingContext::make(t0,
			       1000.0 /*ref_spot*/,
			       2.5 /*volatility*/,
			       0.05 /*rate (yctx)*/);

      REQUIRE(pricing_cx.get() != nullptr);

      auto model
	= StrikeSetMarketModel::make(optionset,
				     tracer,
				     pricing_cx,
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

      OptionId start_id(0);

      auto optionset
	= OptionStrikeSet::regular(1 /*n*/,
				   start_id,
				   1000.0 /*lo_strike*/,
				   5.0 /*d_strike*/,
				   expiry1_tm,
				   Pxtick::nickel_dime);

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

      auto pricing_cx
	= PricingContext::make(t0,
			       1000.0 /*ref_spot*/,
			       1.25 /*volatility*/,
			       0.05 /*rate (yctx)*/);

      REQUIRE(pricing_cx.get() != nullptr);

      auto model
	= StrikeSetMarketModel::make(optionset,
				     tracer,
				     pricing_cx,
				     std::chrono::seconds(1) /*ul_ev_interval_dt*/);

      REQUIRE(model.get() != nullptr);

      std::uint32_t n_tick = 0;

      auto fn = ([&n_tick]
		 (BboTick const & tk)
      {
	constexpr bool c_logging_enabled = true;
	scope lscope("TEST_CASE(strikeset-market-model-empty):lambda", c_logging_enabled);

	if (c_logging_enabled) {
	  lscope.log("enter",
		     xtag("tk", tk));
	}

	++n_tick;
      });

      model->add_omd_callback(new option::FunctionOmdCb(fn));

      auto simulator
	= Simulator::make(t0);

      REQUIRE(simulator.get() != nullptr);

      model->bind_reactor(simulator.borrow());

      /* run simulation */

      utc_nanos t1 = t0 + minutes(1);

      simulator->run_until(t1);

      /* verify simulation invoked our custom callback */

      /* 1-minute simulation generates ticks, depending on hysteresis  etc */

      REQUIRE(n_tick == 90);
    } /*TEST_CASE(strikeset-market-model-empty)*/

  } /*namespace ut*/

} /*namespace xo*/

/* end StrikeSetMarketModel.cpp */
