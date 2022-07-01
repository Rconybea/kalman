/* @file Greeks.cpp */

#include "Greeks.hpp"

namespace xo {
  namespace option {
    std::int64_t
    GreeksEvent::compare(GreeksEvent const & x,
			 GreeksEvent const & y)
    {
      using xo::time::nanos;

      nanos dt = x.tm() - y.tm();

      if(dt != nanos(0))
	return dt.count();

      /* timestamps equal -> compare ids */
      return OptionId::compare(x.oid(), y.oid());
    } /*compare*/
	
  } /*namespace option*/
} /*namespace xo*/

/* end Greeks.cpp */
