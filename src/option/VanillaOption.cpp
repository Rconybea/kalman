/* @file VanillaOption.cpp */

#include "time/Time.hpp"
#include "option/VanillaOption.hpp"

namespace xo {
  using logutil::tostr;
  using logutil::xtag;

  namespace option {
    std::string
    VanillaOption::display_string() const
    {
      return tostr("<VanillaOption",
		   xtag("id", id_),
		   xtag("callput", callput_),
		   xtag("strike", strike_),
		   xtag("expiry", expiry_),
		   xtag("pxtick", pxtick_),
		   ">");
    } /*display_string*/
  } /*namespace option*/
} /*namespace xo*/

/* end VanillaOption.cpp */
