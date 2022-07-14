/* @file Source.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"

namespace xo {
  namespace reactor {
    /* abstract api for a source of events.
     * Event representation is left open:  Sources and Sinks
     * need to have compatible event representations,
     * and coordination is left to such (Source, Sink) pairs.
     */
    class Source : public ref::Refcount {
    }; /*Source*/

    using SourcePtr = ref::rp<Source>;
  } /*namespace reactor*/
} /*namespace xo*/

/* end Source.hpp */
