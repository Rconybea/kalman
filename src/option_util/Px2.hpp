/* @file Px2.hpp */

#pragma once

#include "option_util/Price.hpp"
#include "option_util/Side.hpp"
#include "logutil/fixed.hpp"

namespace xo {
  namespace option {
    /* a bid/ask pair */
    class Px2 {
    public:
      Px2() = default;
      Px2(Price bid, Price ask) : bid_px_{bid}, ask_px_{ask} {}
      
      static bool equals(Px2 const & x, Px2 const & y) {
	return (x.bid_px() == y.bid_px()) && (x.ask_px() == y.ask_px());
      } /*equals*/

      Price bid_px() const { return bid_px_; }
      Price ask_px() const { return ask_px_; }
      Price px(Side s) const {
	switch(s) {
	case Side::bid:
	  return this->bid_px_;
	case Side::ask:
	  return this->ask_px_;
	case Side::end:
	  return Price();
	}
      } /*px*/

      Price spread() const { return this->ask_px_ - this->bid_px_; }

      bool fades(Side s, Px2 const & px2) const {
	return side_compare_px(s, bid_px_, px2.bid_px()) < 0;
      }

      Px2 & assign_bid_px(Price x) { this->bid_px_ = x; return *this; }
      Px2 & assign_ask_px(Price x) { this->ask_px_ = x; return *this; }

      Px2 & assign_px(Side s, Price x) {
	switch(s) {
	case Side::bid:
	  this->bid_px_ = x;
	  break;
	case Side::ask:
	  this->ask_px_ = x;
	  break;
	case Side::end:
	  break;
	}
	return *this;
      } /*assign_px*/

      Px2 & assign_px(Side s, Px2 const & px2) {
	return this->assign_px(s, px2.px(s));
      } /*assign_px*/

    private:
      Price bid_px_;
      Price ask_px_;
    }; /*Px2*/

    inline int32_t side_compare_px(Side s, Px2 const & x, Px2 const & y) {
      return side_compare_px(s, x.px(s), y.px(s));
    } /*side_compare_px*/

    inline bool side_matches_or_improves_px(Side s, Px2 const & x, Px2 const & y) {
      return side_matches_or_improves_px(s, x.px(s), y.px(s));
    } /*side_matches_or_improves_px*/

    inline bool operator==(Px2 const & x, Px2 const & y) {
      return Px2::equals(x, y);
    }

    inline bool operator!=(Px2 const & x, Px2 const & y) {
      return !Px2::equals(x, y);
    } /*operator!=*/

    inline std::ostream & operator<<(std::ostream & os,
				     Px2 const & px2)
    {
      using logutil::fixed;

      os << fixed(px2.bid_px().to_double(), 2) << "-" << fixed(px2.ask_px().to_double(), 2);
      return os;
    } /*operator<<*/

  } /*namespace option*/
} /*namespace xo*/

/* end Px2.hpp */

