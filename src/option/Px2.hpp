/* @file Px2.hpp */

#pragma once

#include "option/Price.hpp"

namespace xo {
  namespace option {
    /* a bid/ask pair */
    class Px2 {
    public:
      Px2() = default;
      Px2(Price bid, Price ask) : bid_px_{bid}, ask_px_{ask} {}
      
      Price bid_px() const { return bid_px_; }
      Price ask_px() const { return ask_px_; }

    private:
      Price bid_px_;
      Price ask_px_;
    }; /*Px2*/
  } /*namespace option*/
} /*namespace xo*/
