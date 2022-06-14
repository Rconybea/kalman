/* @file Reactor.hpp */

#pragma once

#include <cstdint>

namespace xo {
  namespace reactor {
    class Source;

    /* abtract api for a reactor:
     * something that arranges to have work done on a set of Sources.
     */
    class Reactor {
    public:
      virtual ~Reactor() = default;
      
      /* add source src to this reactor.
       * on success, invoke src.notify_reactor_add(this)
       */
      virtual void add_source(Source * src) = 0;

      /* remove source src from this reactor.
       * source must previously have been added by
       * .add_source(src).
       *
       * on success, invoke src.notify_reactor_remove(this)
       */
      virtual void remove_source(Source * src) = 0;

      /* dispatch one reactor event,  borrowing the calling thread
       * amount of work this represents is Source/Sink specific.
       *
       * returns #of events dispatched (0 or 1)
       */
      virtual std::uint64_t run_one() = 0;
    }; /*Reactor*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Reactor.hpp */
