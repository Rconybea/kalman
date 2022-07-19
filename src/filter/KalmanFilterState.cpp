/* @file KalmanFilterState.cpp */

#include "KalmanFilterState.hpp"
#include "print_eigen.hpp"
#include "logutil/scope.hpp"
#include "Eigen/src/Core/Matrix.h"

namespace xo {
  using xo::time::utc_nanos;
  using logutil::matrix;
  using logutil::scope;
  using logutil::xtag;
  using Eigen::LDLT;
  using Eigen::MatrixXd;
  using Eigen::VectorXd;

  namespace kalman {
    KalmanFilterState::KalmanFilterState() = default;

    KalmanFilterState::KalmanFilterState(uint32_t k,
					 utc_nanos tk,
					 VectorXd x,
					 MatrixXd P)
      : k_{k}, tk_{tk}, x_{std::move(x)}, P_{std::move(P)}
    {}

    // ----- KalmanFilterExt -----

    KalmanFilterStateExt::KalmanFilterStateExt(uint32_t k,
					       utc_nanos tk,
					       VectorXd x,
					       MatrixXd P,
					       MatrixXd K,
					       int32_t j)
      : KalmanFilterState(k, tk, x, P),
	j_{j},
	K_{std::move(K)}
    {
      uint32_t n = x.size();

      if (n != P.rows() || n != P.cols()) {
	std::string err_msg
	  = tostr("with n=x.size expect [n x n] covar matrix P",
		  xtag("n", x.size()),
		  xtag("P.rows", P.rows()),
		  xtag("P.cols", P.cols()));

	  throw std::runtime_error(err_msg);
      }

      if ((K.rows() > 0) && (K.rows() > 0)) {
	if (n != K.rows()) {
	  std::string err_msg
	    = tostr("with n=x.size expect [m x n] gain matrix K",
		    xtag("n", x.size()),
		    xtag("K.rows", K.rows()),
		    xtag("K.cols", K.cols()));

	  throw std::runtime_error(err_msg);
	}
      } else {
	/* bypass test with [0 x 0] matrix K;
	 * normal for initial filter state
	 */
      }
    } /*ctor*/

    void
    KalmanFilterState::display(std::ostream & os) const
    {
      os << "<KalmanFilterState"
	 << xtag("step", k_)
	 << xtag("tm", tk_)
	 << xtag("x", x_)
	 << xtag("P", matrix(P_))
	 << ">";
    } /*display*/

    std::string
    KalmanFilterState::display_string() const
    {
      std::stringstream ss;
      ss << *this;
      return ss.str();
    } /*display_string*/
  } /*namespace filter*/
} /*namespace xo*/

/* end KalmanFilterState.cpp */
