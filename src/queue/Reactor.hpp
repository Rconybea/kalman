/* @file Reactor.hpp */

#pragma once

#include "refcnt/Refcounted.hpp"
#include <cstdint>

namespace xo {
  namespace reactor {
    class ReactorSource;

    /* abtract api for a reactor:
     * something that arranges to have work done on a set of Sources.
     */
    class Reactor : public ref::Refcount {
    public:
      virtual ~Reactor() = default;
      
      /* notification when non-primed source (source with no known events)
       * becomes primed (source with at least one event)
       */
      virtual void notify_source_primed(ref::brw<ReactorSource> src) = 0;

      /* add source src to this reactor.
       * on success, invoke src.notify_reactor_add(this)
       *
       * returns true if source added;  false if already present
       */
      virtual bool add_source(ref::brw<ReactorSource> src) = 0;

      /* remove source src from this reactor.
       * source must previously have been added by
       * .add_source(src).
       *
       * on success, invoke src.notify_reactor_remove(this)
       *
       * returns true if source removed;  false if not present
       */
      virtual bool remove_source(ref::brw<ReactorSource> src) = 0;

      /* dispatch one reactor event,  borrowing the calling thread
       * amount of work this represents is Source/Sink specific.
       *
       * returns #of events dispatched (0 or 1)
       */
      virtual std::uint64_t run_one() = 0;

      /* run indefinetly,  borrowing calling thread */
      void run() {
	for(;;) {
	  this->run_one();
	}
      } /*run*/
    }; /*Reactor*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Reactor.hpp */
