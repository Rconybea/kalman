/* @file PollingReactor.hpp */

#pragma once

#include "queue/Reactor.hpp"
#include "queue/Source.hpp"
#include <vector>
#include <cstdint>

namespace xo {
  namespace reactor {
    /* reactor that runs by polling an ordered set of sources */
    class PollingReactor : public Reactor {
    public:
      PollingReactor() = default;

      // ----- inherited from Reactor -----

      virtual bool add_source(ref::brw<ReactorSource> src) override;
      virtual bool remove_source(ref::brw<ReactorSource> src) override;
      virtual std::uint64_t run_one() override;

    private:
      /* find non-empty source,  starting from .source_v_[start_ix],
       * wrapping around to .source_v_[start_ix - 1].
       *
       * return index of first available non-empty source,
       * or -1 if all sources are empty
       */
      std::int64_t find_nonempty_source(std::size_t start_ix);

    private:
      /* next source to poll will be .source_v_[.next_ix_] */
      std::size_t next_ix_ = 0;

      /* ordered set of sources (see reactor::Source)
       * reactor will poll sources in round-robin order 
       */
      std::vector<ReactorSourcePtr> source_v_;
    }; /*PollingReactor*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end PollingReactor.hpp */
