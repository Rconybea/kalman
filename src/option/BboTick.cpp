/* @file BboTick.cpp */

#include "BboTick.hpp"
#include "logutil/tag.hpp"

namespace xo {
  using logutil::xtag;

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

    void
    BboTick::display(std::ostream & os) const {
      os << "{bbo-tick"
	 << xtag("tm", this->tm())
	 << xtag("id", this->id())
	 << xtag("pxz2", this->pxz2())
	 << "}";
    } /*display*/
  } /*namespace option*/
} /*namespace xo*/

/* end BboTick.cpp */
