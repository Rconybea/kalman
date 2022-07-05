/* @file NotifyStrikeSetOmd.hpp */

#include "option/OmdCallback.hpp"
#include "option/StrikeSetOmd.hpp"

namespace xo {
  namespace option {
    /* callback for updating a StrikeSetOmd instance from tick data */
    class UpdateStrikeSetOmd : public OmdCallback {
    public:
      // ----- inherited from OmdCallback -----

      virtual void notify_bbo(BboTick const & bbo_tick) override {
	this->ss_omd_->notify_bbo(bbo_tick);
      } /*notify_bbo*/

      /* invoked once when this callback is attached to a market data source */
      virtual void notify_add_callback() override {}
      /* invoked once when this callback is detached from a market data source */
      virtual void notify_remove_callback() override {}
      
    private:
      /* update on incoming bbo tick */
      ref::rp<StrikeSetOmd> ss_omd_;
    }; /*UpdateStrikeSetOmd*/


  } /*namespace option*/
} /*namespace xo*/

/* end NotifyStrikeSetOmd.hpp */
