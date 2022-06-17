/* @file StrikeSetMarketModel.cpp */

#include "StrikeSetMarketModel.hpp"
#include "process/RealizationSimSource.hpp"

namespace xo {
  using xo::process::RealizationTracer;
  using xo::process::RealizationSimSource;
  using xo::sim::SimulationSource;
  using xo::ref::rp;
  using xo::time::utc_nanos;

  namespace option {
    namespace {
      /* forward underlying price updates to a StrikeSetMarketModel instance */
      class NotifyMarketModel {
      public:
	NotifyMarketModel() = default;

	void assign_model(StrikeSetMarketModel * p) {
	  this->p_mkt_model_ = p;
	}

	void operator()(std::pair<utc_nanos, double> const & ul_px) const {
	  StrikeSetMarketModel * p = this->p_mkt_model_;

	  if(p)
	    p->notify_ul(ul_px);
	} /*operator()*/

      private:
	StrikeSetMarketModel * p_mkt_model_ = nullptr;
      }; /*NotifyMarketModel*/
    } /*namespace*/

    ref::rp<StrikeSetMarketModel>
    StrikeSetMarketModel::make(ref::rp<OptionStrikeSet> option_set,
			       ref::rp<RealizationTracer<double>> ul_tracer,
			       nanos ul_ev_interval_dt)
    {
      /* sim source for underlying prices.
       * feeds updates to *this
       */
      rp<RealizationSimSource<double, NotifyMarketModel>> src
	= RealizationSimSource<double, NotifyMarketModel>::make(ul_tracer,
								ul_ev_interval_dt,
								NotifyMarketModel());
								
      rp<StrikeSetMarketModel> retval
	(new StrikeSetMarketModel(std::move(option_set),
				  std::move(ul_tracer)));

      src->ev_sink_addr()->assign_model(retval.get());

      return retval;
    } /*make*/

    void
    StrikeSetMarketModel::notify_ul(std::pair<utc_nanos, double> const & ul_px)
    {
      XO_STUB();
    } /*update_ul*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetMarketModel.cpp */
