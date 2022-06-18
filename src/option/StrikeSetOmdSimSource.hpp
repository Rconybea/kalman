/* @file StrikeSetOmdSimSource.hpp */

#include "simulator/SimulationSource.hpp"

namespace xo {
  namespace option {
    /* source for fabricated option market data events
     * provided by StrikeSetMarketModel / OptionMarketModel
     */
    class StrikeSetOmdSimSource : public sim::SimulationSource {
    public:
      ref::rp<StrikeSetOmdSimSource> make() { return new StrikeSetOmdSimSource(); }
      
      // ----- inherited from SimulationSource -----

      virtual bool is_exhausted() const override;
      virtual utc_nanos current_tm() const override;
      virtual void advance_until(utc_nanos tm, bool replay_flag) override;
      virtual std::uint64_t advance_one() override;

    private:
      StrikeSetOmdSimSource() = default;

    private:
      /* collection of events */
    }; /*StrikeSetOmdSimSource*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmdSimSource.hpp */
