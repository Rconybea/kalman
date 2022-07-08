/* @file StrikeSetOmd.hpp */

#pragma once

#include "time/Time.hpp"
#include "refcnt/Refcounted.hpp"
#include "option/OptionStrikeSet.hpp"
#include "option/BboTick.hpp"
#include "option_util/PxSize2.hpp"
#include <array>

namespace xo {
  namespace option {
    /* market data for a particular option
     */
    class Omd {
    public:
      using utc_nanos = time::utc_nanos;

    public:
      Omd() = default;

      utc_nanos tm(Side s) const { return this->tm_v_[side2int(s)]; }
      Size size(Side s) const { return this->bbo_pxz2_.size(s); }
      Price px(Side s) const { return this->bbo_pxz2_.px(s); }

      bool is_bid_present() const { return bbo_pxz2_.is_bid_present(); }
      bool is_ask_present() const { return bbo_pxz2_.is_ask_present(); }
      
      /* update state for new market data event */
      void notify_bbo(BboTick const & bbo_tick);

    private:
      /* last timestamps for bid/offer respectively */
      std::array<utc_nanos, 2> tm_v_;
      /* top of book (local best bid/offer) */
      PxSize2 bbo_pxz2_;
    }; /*Omd*/

    /* market data for a (call, put) pair (i.e. all other terms shared) */
    class OmdPair : public std::array<Omd, 2> {
    public:
      OmdPair() = default;

      /* update state for incoming market data tick */
      void notify_bbo(BboTick const & bbo_tick);
    }; /*OmdPair*/

    /* option market data for a set of related options
     * - collects and maintains current prices from streaming option market data 
     * - also collects and maintains spot price
     */
    class StrikeSetOmd : public ref::Refcount
    {
    public:
      using utc_nanos = time::utc_nanos;

      /* create instance */
      static ref::rp<StrikeSetOmd> make(ref::rp<OptionStrikeSet> const & oset);

      /* lookup current state for a particular option */
      Omd const & lookup(OptionId id) const;
      Omd & lookup(OptionId id);

      /* notification on incoming bbo tick;
       * update .omd_v[] for corresponding option
       */
      void notify_bbo(BboTick const & bbo_tick);

    private:
      StrikeSetOmd(ref::rp<OptionStrikeSet> const & oset);

    private:
      /* option terms for each option assoc'd with this object */
      ref::rp<OptionStrikeSet> option_set_;
      /* the i'th strike in .option_set associates with the i'th strike in .omd_v[] */
      std::vector<OmdPair> omd_v_;
    }; /*StrikeSetOmd*/

  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmd.hpp */
