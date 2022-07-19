/* @file KalmanFilterTransition.hpp */

#pragma once

#include "time/Time.hpp"
#include <Eigen/Dense>
#include <cstdint>

namespace xo {
  namespace kalman {

    /* encapsulate transition behavior for a kalman filter
     * before taking observations into account
     */
    class KalmanFilterTransition {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using uint32_t = std::uint32_t;

    public:
      KalmanFilterTransition() = default;
      KalmanFilterTransition(MatrixXd F,
			     MatrixXd Q)
	: F_{std::move(F)}, Q_{std::move(Q)} { assert(this->check_ok()); }

      /* n:    cardinality of state vector */
      uint32_t n_state() const;

      MatrixXd const & transition_mat() const { return F_; }
      MatrixXd const & transition_cov() const { return Q_; }

      bool check_ok() const {
	uint32_t n = F_.rows();
	bool f_is_nxn = ((F_.rows() == n) && (F_.cols() == n));
	bool q_is_nxn = ((Q_.rows() == n) && (Q_.cols() == n));

	/* also would like to require: Q is +ve definite */

	return f_is_nxn && q_is_nxn;
      } /*check_ok*/

    private:
      /* [n x n] state transition matrix */
      MatrixXd F_;
      /* [n x n] covariance matrix for system noise */
      MatrixXd Q_;
    }; /*KalmanFilterTransition*/
  } /*namespace kalman*/
} /*namespace xo*/
    
/* end KalmanFilterTransition.hpp */
