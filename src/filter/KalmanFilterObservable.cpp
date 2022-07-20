/* @file KalmanFilterObservable.cpp */

#include "KalmanFilterObservable.hpp"
#include "print_eigen.hpp"
#include "logutil/scope.hpp"

namespace xo {
  using logutil::matrix;
  using logutil::xtag;

  namespace kalman {
    void
    KalmanFilterObservable::display(std::ostream & os) const
    {
      os << "<KalmanFilterObservable"
	 << xtag("H", matrix(H_))
	 << xtag("R", matrix(R_))
	 << ">";
    } /*display*/

    std::string
    KalmanFilterObservable::display_string() const
    {
      std::stringstream ss;
      this->display(ss);
      return ss.str();
    } /*display_string*/

  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterObservable.cpp */
