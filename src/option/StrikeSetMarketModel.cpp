/* @file StrikeSetMarketModel.cpp */

#include "time/Time.hpp" /*need this before xtag decl for some reason*/
#include "StrikeSetMarketModel.hpp"
#include "BlackScholes.hpp"
#include "process/RealizationSimSource.hpp"
#include "queue/Reactor.hpp"

namespace xo {
  using xo::process::RealizationTracer;
  using xo::process::RealizationSimSource;
  using xo::sim::SimulationSource;
  using xo::reactor::Reactor;
  using xo::ref::rp;
  using xo::ref::brw;
  using xo::time::utc_nanos;
  using logutil::scope;
  using logutil::xtag;

  namespace option {
    void
    OptionMarketModel::notify_ul(std::pair<utc_nanos, double> const & ul_ev,
				 ref::brw<PricingContext> ul_pricing_cx)
    {
      bool c_logging_enabled = true;
      scope lscope("OptionMarketModel", "::notify_ul", c_logging_enabled);

      this->last_greeks_ = BlackScholes::greeks(this->option_.borrow(),
						ul_pricing_cx,
						ul_ev.second /*ul_spot*/,
						ul_ev.first /*t0*/);

      /* apply an arbitrary spread around tv.  $0.02/share = $2/contract */
      constexpr double c_half_spread = 0.02; /* FIXME: parameterize */

      double model_bid1
	= this->option_->sh2px(this->last_greeks_.tv() - c_half_spread);
      double model_ask1
	= this->option_->sh2px(this->last_greeks_.tv() + c_half_spread);

      Price model_bid
	= PxtickUtil::glb_tick(this->option_->pxtick(), model_bid1);
      Price model_ask
	= PxtickUtil::lub_tick(this->option_->pxtick(), model_ask1);

      this->last_bbo_px2_ = Px2(model_bid, model_ask);

      if (c_logging_enabled)
	lscope.log("enter",
		   xtag("tm", ul_ev.first),
		   xtag("upx", ul_ev.second),
		   xtag("callput", (this->option_->callput() == Callput::call) ? "C" : "P"),
		   xtag("strike", this->option_->effective_strike()),
		   xtag("tv", this->last_greeks_.tv()),
		   xtag("delta", this->last_greeks_.delta()),
		   xtag("bid", model_bid.to_double()),
		   xtag("ask", model_ask.to_double())
		   );

    } /*notify_ul*/

  namespace {
  /* forward underlying price updates to a StrikeSetMarketModel instance */
  class NotifyMarketModel {
  public:
    NotifyMarketModel() = default;

    void assign_model(StrikeSetMarketModel *p) { this->p_mkt_model_ = p; }

    void operator()(std::pair<utc_nanos, double> const &ul_px) const {
      StrikeSetMarketModel *p = this->p_mkt_model_;

      if (p)
        p->notify_ul(ul_px);
    } /*operator()*/

  private:
    StrikeSetMarketModel *p_mkt_model_ = nullptr;
      }; /*NotifyMarketModel*/
      }  /*namespace*/

    ref::rp<StrikeSetMarketModel>
    StrikeSetMarketModel::make(ref::rp<OptionStrikeSet> option_set,
			       ref::rp<RealizationTracer<double>> ul_tracer,
			       ref::rp<PricingContext> ul_pricing_cx,
			       nanos ul_ev_interval_dt)
    {
      /* sim source for underlying prices.
       * feeds updates to *this
       */
      rp<RealizationSimSource<double, NotifyMarketModel>> ul_sim_src
	= RealizationSimSource<double, NotifyMarketModel>::make(ul_tracer,
								ul_ev_interval_dt,
								NotifyMarketModel());
								
      rp<StrikeSetMarketModel> retval
	(new StrikeSetMarketModel(std::move(option_set),
				  std::move(ul_tracer),
				  ul_sim_src,
				  std::move(ul_pricing_cx)));

      ul_sim_src->ev_sink_addr()->assign_model(retval.get());

      return retval;
    } /*make*/

    StrikeSetMarketModel::StrikeSetMarketModel(ref::rp<OptionStrikeSet> option_set,
					       ref::rp<RealizationTracer<double>> ul_realization,
					       ref::rp<SimulationSource> ul_sim_src,
					       ref::rp<PricingContext> ul_pricing_cx)
      : option_set_{std::move(option_set)},
	ul_realization_tracer_{std::move(ul_realization)},
	ul_sim_src_{std::move(ul_sim_src)},
	ul_pricing_cx_{std::move(ul_pricing_cx)}
    {
      this->option_set_->verify_ok(true);

      /* populate .market_v */
      auto visitor_fn
	= ([this]
	   (StrikePair const & k_pair)
	{
	  this->market_v_.push_back(OptionMarketModel(k_pair.call().promote()));
	  this->market_v_.push_back(OptionMarketModel(k_pair.put().promote()));
	});

      this->option_set_->visit_strikes(visitor_fn);
    } /*ctor*/

    void
    StrikeSetMarketModel::notify_ul(std::pair<utc_nanos, double> const & ul_ev)
    {
      bool c_logging_enabled = true;
      scope lscope("StrikeSetMarketModel", "::notify_ul", c_logging_enabled);

      if (c_logging_enabled)
	lscope.log("enter",
		   xtag("tm", ul_ev.first),
		   xtag("px", ul_ev.second));

      /* update option-market models */
      for(OptionMarketModel & opt_mkt : this->market_v_) {
	opt_mkt.notify_ul(ul_ev,
			  this->ul_pricing_cx_.borrow());
      }
    } /*update_ul*/

    void
    StrikeSetMarketModel::bind_reactor(brw<Reactor> reactor)
    {
      assert(reactor.get());

      if(this->ul_sim_src_)
	reactor->add_source(this->ul_sim_src_);
    } /*bind_reactor*/

    void
    StrikeSetMarketModel::detach_reactor(brw<Reactor> reactor)
    {
      assert(reactor.get());

      if(this->ul_sim_src_)
	reactor->remove_source(this->ul_sim_src_);
    } /*detach_reactor*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetMarketModel.cpp */
