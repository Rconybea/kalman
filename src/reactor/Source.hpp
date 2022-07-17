/* @file Source.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include <string>

namespace xo {
  namespace reactor {
    class AbstractSink;

    template<typename T>
    class Sink1;

    /* abstract api for a source of events.
     * Event representation is left open:  Sources and Sinks
     * need to have compatible event representations,
     * and coordination is left to such (Source, Sink) pairs.
     *
     * See ReactorSource, for example
     *
     * Typically a Source will have one or more .add_callback()
     * methods, for listening to source events
     */
    class AbstractSource : public virtual ref::Refcount {
    public:
      virtual void attach_sink(ref::rp<AbstractSink> const & sink) = 0;
      virtual void detach_sink(ref::rp<AbstractSink> const & sink) = 0;

      /* human-readable string identifying this source */
      virtual std::string display_string() const;
    }; /*AbstractSource*/

    using AbstractSourcePtr = ref::rp<AbstractSource>;

    /* Source for events of type T.
     * Can connect to a Sink1<T>
     */
    template<typename T>
    class Source1 : public AbstractSource {
    public:
      // ----- inherited from AbstractSource -----
    }; /*Source1*/

  } /*namespace reactor*/
} /*namespace xo*/

/* end Source.hpp */
