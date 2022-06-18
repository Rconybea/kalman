/* @file StrikeSetMarketModel.hpp */

#include "option/VanillaOption.hpp"
#include "option/PricingContext.hpp"
#include "option/OptionStrikeSet.hpp" 
#include "option/PricingContext.hpp"
#include "option/Greeks.hpp"
#include "process/RealizationTracer.hpp"
#include "simulator/SimulationSource.hpp"

namespace xo {
  namespace option {
    /* model market for a particular option.
     *
     * 1. generate model values (tv) for .option,
     *    based on option pricing model,
     * 2. generate suitable bid/ask at model spread,
     *    widening to whole tick multiple.
     * 3. generate model market datastream,  apply hysteresis
     */
    class OptionMarketModel {
    public:
      using utc_nanos = xo::time::utc_nanos;

    public:
      OptionMarketModel() = default;
      explicit OptionMarketModel(ref::rp<VanillaOption> option)
	: option_{std::move(option)} {}

      ref::brw<VanillaOption> option() const { return option_; }

      void notify_ul(std::pair<utc_nanos, double> const & ul_ev,
		     ref::brw<PricingContext> ul_pricing_cx);

    private:
      /* providing market model for this option */
      ref::rp<VanillaOption> option_;

      /* greeks asof last call to .notify_ul() */
      Greeks last_greeks_;
    }; /*OptionMarketModel*/

    /* model market, for a set of related options with similar terms
     * and shared expiry
     */
    class StrikeSetMarketModel : public ref::Refcount {
    public:
      using SimulationSource = xo::sim::SimulationSource;
      using Reactor = reactor::Reactor;
      using utc_nanos = xo::time::utc_nanos;
      using nanos = xo::time::nanos;

      template<typename T>
      using RealizationTracer = xo::process::RealizationTracer<T>;
      
    public:
      /* create model instance.
       * - option_set.  model market for this set of related options
       * - ul_model.    model for timeseries of underlying prices.
       *                (package as sim source feeding updates to StrikeSetMarketModel)
       * - ul_pricing_cx.       option pricing inputs (rates, divs, volsfc, ..)
       * - ul_ev_interval_dt.   underlying prices will change with this regular period
       */
      static ref::rp<StrikeSetMarketModel> make(ref::rp<OptionStrikeSet> option_set,
						ref::rp<RealizationTracer<double>> ul_tracer,
						ref::rp<PricingContext> ul_pricing_cx,
						nanos ul_ev_interval_dt);

      /* notify option market-model on underlying price change */
      void notify_ul(std::pair<utc_nanos, double> const & ul_ev);

      /* add this models' sources to reactor:
       * for example reactor will trigger events that flow into .notify_ul()
       */
      void bind_reactor(ref::brw<Reactor> reactor);

      /* reverse the effect of .bind_reactor() */
      void detach_reactor(ref::brw<Reactor> reactor);

    private:
      StrikeSetMarketModel(ref::rp<OptionStrikeSet> option_set,
			   ref::rp<RealizationTracer<double>> ul_realization,
			   ref::rp<SimulationSource> ul_sim_src,
			   ref::rp<PricingContext> ul_pricing_cx);

    private:
      /* option terms for this market */
      ref::rp<OptionStrikeSet> option_set_;

      /* pricing context -- non-terms inputs to option pricing model */
      ref::rp<RealizationTracer<double>> ul_realization_tracer_;

      /* simulation source for underlying prices */
      ref::rp<SimulationSource> ul_sim_src_;

      /* pricing context for options on this underlying */
      ref::rp<PricingContext> ul_pricing_cx_;

      /* model for each option in .option_set
       * model includes invented market prices and greeks
       */
      std::vector<OptionMarketModel> market_v_;

    }; /*StrikeSetMarketModel*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetMarketModel.hpp */

