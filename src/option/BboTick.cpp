/* @file BboTick.cpp */

#include "BboTick.hpp"

namespace xo {
  namespace option {
      static int64_t compare(BboTick const & x,
			     BboTick const & y)
      {
	using xo::time::nanos;

	nanos dt = x.tm() - y.tm();

	if(dt != nanos(0))
	  return dt.count();

	/* timestamps equal -> compare ids */
	return OptionId::compare(x.id(), y.id());
      } /*compare*/
  } /*namespace option*/
} /*namespace xo*/

/* end BboTick.cpp */
