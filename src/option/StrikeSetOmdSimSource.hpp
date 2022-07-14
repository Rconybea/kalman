/* @file StrikeSetOmdSimSource.hpp */

#pragma once

#include "queue/SecondarySource.hpp"
#include "option/OmdCallback.hpp"

namespace xo {
  namespace option {
    using StrikeSetOmdSimSource = xo::reactor::SecondarySource<BboTick, OmdCallback, &OmdCallback::notify_bbo>;
  } /*namespace option*/
} /*namespace xo*/

#ifdef OBSOLETE
#include "simulator/SimulationSource.hpp"
#include "option/OmdCallback.hpp"
#include "option/BboTick.hpp"
#include "callback/CallbackSet.hpp"
#include <vector>

namespace xo {
  namespace option {
    /* source for fabricated option market data events
     * provided by StrikeSetMarketModel / OptionMarketModel
     */
    class StrikeSetOmdSimSource : public sim::SimulationSource {
    public:
      template<typename Fn>
      using CallbackSet = fn::CallbackSet<Fn>;

    public:
      static ref::rp<StrikeSetOmdSimSource> make() { return new StrikeSetOmdSimSource(); }
      
      /* flag upstream as exhausted:
       * .notify_bbo() should not be called after .notify_upstream_exhausted()
       */
      void notify_upstream_exhausted();

      /* send a marketdata tick to OmdSimSource;
       * source will forward to attached callbacks
       */
      void notify_bbo(BboTick const & tick);

      template<typename T>
      void notify_bbo_v(T const & v) {
	for(BboTick const & tk : v)
	  this->notify_bbo(tk);
      } /*notify_bbo_v*/

      /* invoke cb->notify_bbo() for each tick */
      void add_callback(ref::rp<OmdCallback> const & cb);
      /* drop cb from this source's callback set */
      void remove_callback(ref::rp<OmdCallback> const & cb);

      // ----- inherited from SimulationSource -----

      virtual bool is_empty() const override;
      virtual bool is_exhausted() const override;
      virtual utc_nanos current_tm() const override;
      virtual std::uint64_t advance_until(utc_nanos tm, bool replay_flag) override;

      // ----- inherited from Source -----

      virtual std::uint64_t deliver_one() override;
      /* note: native pointer to avoid cycle */
      virtual void notify_reactor_add(Reactor * reactor) override;
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

      /* may set this to true, just once, to announce that upstream
       * will send no more events.
       * see .notify_upstream_exhausted()
       */
      bool upstream_exhausted_ = false;

      /* queued events to be delivered to reactor */
      std::vector<BboTick> omd_heap_;

      /* reactor being used to schedule consumption from this source */
      Reactor * parent_reactor_ = nullptr;

      /* invoke callbacks in this set for each marketdata event */
      CallbackSet<ref::rp<OmdCallback>> cb_set_;
    }; /*StrikeSetOmdSimSource*/
  } /*namespace option*/
} /*namespace xo*/
#endif

/* end StrikeSetOmdSimSource.hpp */
