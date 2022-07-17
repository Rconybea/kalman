/* @file OmdCallback.hpp */

#pragma once

#include "reactor/Sink.hpp"
#include "option/BboTick.hpp"

namespace xo {
  namespace option {
    /* callback for consuming market data */
    using OmdCallback = reactor::Sink1<BboTick>;

    /* callback for consuming market data,
     * wrapping a std::function<>
     */
    using FunctionOmdCb = reactor::SinkToFunction
      <BboTick,
       std::function<void (BboTick const &)>>;

  } /*namespace option*/
} /*namespace xo*/

/* end OmdCallback.hpp */
