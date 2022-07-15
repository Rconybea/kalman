/* @file SourceTimestamp.hpp */

#include "reactor/ReactorSource.hpp"

namespace xo {
  namespace sim {
    /* remember a timestamp for a simulation source;
     * use to insert a source into simulation heap.
     * don't want to use SimulationSource.t0,  so that we can 
     * promise heap invariants without reying on
     * any behavior of SimulationSource.
     *
     * Note: Need to resolve ties between different sources,
     *       if they coincide on timestamp of next event.
     *       For now use SimulationSource address
     */
    class SourceTimestamp {
    public:
      using ReactorSource = xo::reactor::ReactorSource;
      using utc_nanos = xo::time::utc_nanos;
      
    public:
      SourceTimestamp(utc_nanos t0,
		      ReactorSource * src)
	: t0_(t0), src_(src) {}
      
      static int32_t compare(SourceTimestamp const & x,
			     SourceTimestamp const & y) {
	using xo::time::utc_nanos;
	using xo::time::nanos;

	nanos dt = x.t0_ - y.t0_;

	if(dt < nanos(0))
	  return -1;
	else if(dt > nanos(0))
	  return +1;
	
	/* timestamps are equal */

	std::ptrdiff_t dptr = (x.src() - y.src());

	return dptr;
      } /*compare*/

      utc_nanos t0() const { return t0_; }
      ReactorSource * src() const { return src_; }

    private:
      /* timestamp for this source */
      utc_nanos t0_;
      /* simulation source
       * promise:
       * - src.t0() >= .t0 || src.is_exhausted
       */
      ReactorSource * src_ = nullptr;
    }; /*SourceTimestamp*/

    inline bool operator==(SourceTimestamp const & x,
			   SourceTimestamp const & y)
    {
      return SourceTimestamp::compare(x, y) == 0;
    } /*operator==*/

    inline bool operator<(SourceTimestamp const & x,
			  SourceTimestamp const & y)
    {
      return SourceTimestamp::compare(x, y) < 0;
    } /*operator<*/

    inline bool operator<=(SourceTimestamp const & x,
			   SourceTimestamp const & y)
    {
      return SourceTimestamp::compare(x, y) <= 0;
    } /*operator<=*/

    inline bool operator>(SourceTimestamp const & x,
			  SourceTimestamp const & y)
    {
      return SourceTimestamp::compare(x, y) > 0;
    } /*operator>*/

    inline bool operator>=(SourceTimestamp const & x,
			   SourceTimestamp const & y)
    {
      return SourceTimestamp::compare(x, y) >= 0;
    } /*operator>=*/
  } /*namespace sim*/
} /*namespace xo*/

/* end SourceTimestamp.hpp*/
