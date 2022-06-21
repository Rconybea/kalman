/* @file StrikeSetOmdSimSource.hpp */

#pragma once

#include "simulator/SimulationSource.hpp"
#include "option/BboTick.hpp"
#include <vector>

namespace xo {
  namespace option {
    /* source for fabricated option market data events
     * provided by StrikeSetMarketModel / OptionMarketModel
     */
    class StrikeSetOmdSimSource : public sim::SimulationSource {
    public:
      static ref::rp<StrikeSetOmdSimSource> make() { return new StrikeSetOmdSimSource(); }
      
      /* flag upstream as exhausted:
       * .notify_bbo() should not be called after .notify_upstream_exhausted()
       */
      void notify_upstream_exhausted();

      void notify_bbo(BboTick const & tick);

      template<typename T>
      void notify_bbo_v(T const & v) {
	for(BboTick const & tk : v)
	  this->notify_bbo(tk);
      } /*notify_bbo_v*/

      // ----- inherited from SimulationSource -----

      virtual bool is_empty() const override;
      virtual bool is_exhausted() const override;
      virtual utc_nanos current_tm() const override;
      virtual std::uint64_t advance_until(utc_nanos tm, bool replay_flag) override;

      // ----- inherited from Source -----

      virtual std::uint64_t deliver_one() override;
      /* note: native pointer to avoid cycle */
      virtual void notify_reactor_add(Reactor * /*reactor*/) override;
      virtual void notify_reactor_remove(Reactor * /*reactor*/) override {}

    private:
      StrikeSetOmdSimSource() = default;

      /* like deliver_one(), but suppress publishing event
       * when replay_flag=false
       */
      uint64_t deliver_one_aux(bool replay_flag);

    private:
      /* current time for this source */
      utc_nanos current_tm_;

      /* set irrevocably to true once upstream announces
       * it will send no more events
       */
      bool upstream_exhausted_ = false;

      /* queued events to be delivered to reactor */
      std::vector<BboTick> omd_heap_;

      /* reactor being used to schedule consumption from this source */
      Reactor * parent_reactor_ = nullptr;
    }; /*StrikeSetOmdSimSource*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmdSimSource.hpp */
