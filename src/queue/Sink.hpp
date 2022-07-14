/* @file Sink.hpp */

#pragma once

#include "queue/Source.hpp"

namespace xo {
  namespace reactor {
    class Sink : public virtual ref::Refcount {
    public:
      virtual ~Sink() = default;

      /* attach an input source.
       * typically this means calling src.add_callback()
       * with a function thats calls a .notify_xxx() method
       * on this Sink
       */
      virtual void attach_source(ref::rp<Source> src) = 0;
    }; /*Sink*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.hpp */
