/* @file EventSink.hpp */

#pragma once

namespace xo {
  namespace sim {
    /* something that observes (consumes) events of type T.
     * we deliberately hide event sinks from top-level of simulator scaffold,
     * so that we don't have to impose a common event type for T
     */
    template<typename T>
    class EventSink {
    public:
      void operator()(T const & x);
    }; /*EventSink*/
  } /*namespace sim*/
} /*namespace xo*/

/* end EventSink.hpp */
