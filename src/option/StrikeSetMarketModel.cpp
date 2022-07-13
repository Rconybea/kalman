/* @file StrikeSetMarketModel.cpp */

#include "time/Time.hpp" /*need this before xtag decl for some reason*/
#include "StrikeSetMarketModel.hpp"
#include "StrikeSetOmdSimSource.hpp"
#include "BlackScholes.hpp"
#include "BboTick.hpp"
#include "process/RealizationSimSource.hpp"
#include "queue/Reactor.hpp"
#include <vector>

namespace xo {
  using xo::process::RealizationTracer;
  using xo::process::RealizationSimSource;
  using xo::reactor::Source;
  using xo::reactor::Reactor;
  using xo::ref::rp;
  using xo::ref::brw;
  using xo::time::utc_nanos;
  using logutil::scope;
  using logutil::xtag;

  namespace option {
    void
    OptionMarketModel::notify_ul(std::pair<utc_nanos, double> const & ul_ev,
				 ref::brw<PricingContext> ul_pricing_cx,
				 std::vector<BboTick> * p_omd_tick_v)
    {
      /* PARAMETERS:
       *   c_half_spread
       *   c_max_spread
       */

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

      Px2 model_px2(PxtickUtil::glb_tick(this->option_->pxtick(), model_bid1),
		    PxtickUtil::lub_tick(this->option_->pxtick(), model_ask1));

      Px2 old_bbo_px2 = this->last_bbo_px2_;

      Px2 new_inside_px2 = model_px2;

      /* apply some hysteresis,  as a function of delta.
       * absent an explicit volume model,  we assume that:
       * 1. high-delta options don't trade (low volume)
       * 2. market-makers minimize updates for options that don't trade
       *    (since they pay for bandwidth,  and need to use it productively)
       */
      Px2 new_bbo_px2 = old_bbo_px2;

      Px2 compete_px2;

      for(Side s : SideIter()) {
	/* consider a price to be competitive if it does not fade w.r.t. this level */
	Price cutoff_px
	  = Price::from_double(this->option_->sh2px(fade_by(s,
							    this->last_greeks_.tv(),
							    1.5 * c_half_spread)));

	compete_px2.assign_px(s, cutoff_px);
      }

      for(Side s : SideIter()) {
	if(new_inside_px2.fades(s, old_bbo_px2)) {
	  /* always publish fades
	   * presume that failing to publish would
	   * promptly trigger opportunistic trade at no-longer-desirable price
	   */
	  new_bbo_px2.assign_px(s, new_inside_px2);
	} else {
	  /* for improves: update as function of delta
	   * - always update for delta < 0.75
	   *   if new price is 'competitive' (within 1.5 * c_half_spread of tv);
	   *   otherwise delay
	   * - always delay update for delta >= 0.75 
	   */
	  
	  if((std::abs(this->last_greeks_.delta()) < 0.75)
	     && side_matches_or_improves_px(s,
					    new_inside_px2,
					    compete_px2))
	  {
	    new_bbo_px2.assign_px(s, compete_px2);
	  }
	}
      }

      constexpr Price c_max_spread = Price::from_double(2.0);

      /* in spite of hysteresis rules,   prevent super-wide quotes;
       * apply max displayed bbo of $2
       */
      if(new_bbo_px2.spread() > c_max_spread) {
	/* refresh to inside price */
	new_bbo_px2 = new_inside_px2;
      } /*if*/
       
      bool publish_flag = (new_bbo_px2 != this->last_bbo_px2_);
      
      this->last_bbo_px2_ = new_bbo_px2;

      if (c_logging_enabled)
	lscope.log("enter",
		   xtag("tm", ul_ev.first),
		   xtag("upx", ul_ev.second),
		   xtag("callput", (this->option_->callput() == Callput::call) ? "C" : "P"),
		   xtag("strike", this->option_->effective_strike()),
		   xtag("tv", this->last_greeks_.tv()),
		   xtag("delta", this->last_greeks_.delta()),
		   xtag("old-bbo-px2", old_bbo_px2),
		   xtag("model-px2", model_px2),
		   xtag("compete-px2", compete_px2),
		   xtag("bbo-px2", new_bbo_px2),
		   xtag("publish", publish_flag)
		   );

      if(publish_flag) {
	/* apply small delay to ul price source */
	utc_nanos omd_tm = ul_ev.first + std::chrono::microseconds(500);

	/* for now fix size at 1 contract
	 * (in lieu of something more sophisticated)
	 */
	BboTick bbo_tick(omd_tm,
			 this->option()->id(),
			 PxSize2::with_size(Size::from_int(1), new_bbo_px2));

	p_omd_tick_v->push_back(bbo_tick);
      }
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
					       ref::rp<Source> ul_sim_src,
					       ref::rp<PricingContext> ul_pricing_cx)
      : option_set_{std::move(option_set)},
	ul_realization_tracer_{std::move(ul_realization)},
	ul_sim_src_{std::move(ul_sim_src)},
	ul_pricing_cx_{std::move(ul_pricing_cx)},
	omd_publisher_{StrikeSetOmdSimSource::make()},
	greeks_publisher_{StrikeSetGreeksSimSource::make()}
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
    StrikeSetMarketModel::add_omd_callback(rp<OmdCallback> const & cb)
    {
      this->omd_publisher_->add_callback(cb);
    } /*add_omd_callback*/

    void
    StrikeSetMarketModel::remove_omd_callback(rp<OmdCallback> const & cb)
    {
      this->omd_publisher_->remove_callback(cb);
    } /*remove_omd_callback*/

    void
    StrikeSetMarketModel::notify_ul_exhausted()
    {
      if (this->omd_publisher_) {
	this->omd_publisher_->notify_upstream_exhausted();
      }
    } /*notify_ul_exhausted*/

    void
    StrikeSetMarketModel::notify_ul(std::pair<utc_nanos, double> const & ul_ev)
    {
      bool c_logging_enabled = true;
      scope lscope("StrikeSetMarketModel", "::notify_ul", c_logging_enabled);

      if (c_logging_enabled)
	lscope.log("enter",
		   xtag("tm", ul_ev.first),
		   xtag("px", ul_ev.second));

      /* TODO: save this across calls to .notify_ul() to avoid re-allocating */
      std::vector<BboTick> omd_tick_v;

      /* update option-market models */
      for(OptionMarketModel & opt_mkt : this->market_v_) {
	opt_mkt.notify_ul(ul_ev,
			  this->ul_pricing_cx_.borrow(),
			  &omd_tick_v);
      }

      if (c_logging_enabled)
	lscope.log("publish",
		   xtag("n-ticks", omd_tick_v.size()));

      /* publish updates in omd_tick_v */
      this->omd_publisher_->notify_event_v(omd_tick_v);
    } /*update_ul*/

    void
    StrikeSetMarketModel::bind_reactor(brw<Reactor> reactor)
    {
      assert(reactor.get());

      if(this->ul_sim_src_)
	reactor->add_source(this->ul_sim_src_);
      if(this->omd_publisher_)
	reactor->add_source(this->omd_publisher_);
    } /*bind_reactor*/

    void
    StrikeSetMarketModel::detach_reactor(brw<Reactor> reactor)
    {
      assert(reactor.get());

      if(this->ul_sim_src_)
	reactor->remove_source(this->ul_sim_src_);
      if(this->omd_publisher_)
	reactor->remove_source(this->omd_publisher_);
    } /*detach_reactor*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetMarketModel.cpp */
