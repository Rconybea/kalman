/* @file BboTick.hpp */

#pragma once

#include "time/Time.hpp"
#include "option/OptionId.hpp"
#include "option_util/PxSize2.hpp"

namespace xo {
  namespace option {
    /* A tick reporting a bbo update for an option.
     * provides separate bid/ask presence,
     * so can report bid-only, ask-only, or bid+ask
     */
    class BboTick {
    public:
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      BboTick(utc_nanos tm, OptionId id, PxSize2 const & pxz2)
	: tm_{tm}, id_{id}, pxz2_{pxz2} {}

      /* compare ticks by timestamp,  then id */
      static int64_t compare(BboTick const & x,
			     BboTick const & y);

      utc_nanos tm() const { return tm_; }
      OptionId id() const { return id_; }
      PxSize2 const & pxz2() const { return pxz2_; }

      bool is_side_present(Side s) const { return pxz2_.is_side_present(s); }
      bool is_bid_present() const { return pxz2_.is_bid_present(); }
      bool is_ask_present() const { return pxz2_.is_ask_present(); }

      void display(std::ostream & os) const;

    private:
      /* timestamp for this tick */
      utc_nanos tm_;
      /* identifies option concerned */
      OptionId id_;
      /* bid/ask price/size
       * invalid size with 0 price if side is not present 
       */
      PxSize2 pxz2_;
    }; /*BboTick*/

    /* consider ticks equal if they have the same instrument + timestamp */
    inline bool operator==(BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) == 0; }
    inline bool operator!=(BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) != 0; }
    inline bool operator< (BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) <  0; }
    inline bool operator<=(BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) <= 0; }
    inline bool operator> (BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) >  0; }
    inline bool operator>=(BboTick const & x, BboTick const & y) { return BboTick::compare(x, y) >= 0; }
      
    inline std::ostream &
    operator<<(std::ostream & os, BboTick const & tk) {
      tk.display(os);
      return os;
    } /*operator<<*/
    
  } /*namespace option*/
} /*namespace xo*/

/* end BboTick.hpp */
