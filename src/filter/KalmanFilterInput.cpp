/* @file KalmanFilterInput.cpp */

#include "KalmanFilterInput.hpp"
#include "print_eigen.hpp"
#include "logutil/scope.hpp"

namespace xo {
  using logutil::matrix;
  using logutil::xtag;

  namespace kalman {
    void
    KalmanFilterInput::display(std::ostream & os) const
    {
      os << "<KalmanFilterInput"
	 << xtag("tkp1", tkp1_)
	 << xtag("z", matrix(z_))
	 << ">";
    } /*display*/

    std::string
    KalmanFilterInput::display_string() const
    {
      std::stringstream ss;
      this->display(ss);
      return ss.str();
    } /*display_string*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInput.cpp */
