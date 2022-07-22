/* @file KalmanFilterStep.cpp */

#include "KalmanFilterStep.hpp"
#include "logutil/scope.hpp"

namespace xo {
  using logutil::tostr;
  using logutil::xtag;

  namespace kalman {
    void
    KalmanFilterStep::display(std::ostream & os) const
    {
      os << "<KalmanFilterStep"
	 << xtag("state", state_)
	 << xtag("model", this->model())
	 << xtag("obs", this->obs())
	 << xtag("input", this->input())
	 << ">";
    } /*display*/

    std::string
    KalmanFilterStep::display_string() const
    {
      return tostr(*this);
    } /*display_string*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterStep.cpp */
