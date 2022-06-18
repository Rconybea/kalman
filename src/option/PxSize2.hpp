/* @file PxSize2.hpp */

#pragma once

#include "option/Price.hpp"
#include "option/Size.hpp"

namespace xo {
  namespace option {
    /* a bid/ask pair with size */
    class PxSize2 {
    public:
      PxSize2() = default;
      PxSize2(Size bid_sz, Price bid_px, Price ask_px, Size ask_sz)
	: bid_sz_{bid_sz}, bid_px_{bid_px}, ask_px_{ask_px}, ask_sz_{ask_sz} {}

      Size bid_sz() const { return bid_sz_; }
      Price bid_px() const { return bid_px_; }
      Price ask_px() const { return ask_px_; }
      Size ask_sz() const { return ask_sz_; }

      bool is_bid_present() const { return bid_sz_.is_valid(); }
      bool is_ask_present() const { return ask_sz_.is_valid(); }

    private:
      Size bid_sz_;
      Price bid_px_;
      Price ask_px_;
      Size ask_sz_;
    }; /*PxSize2*/
  } /*namespace option*/
  } /*namespace xo*/

  /* end PxSize2.hpp */
  
