/* @file KalmanFilterState.hpp */

#pragma once

#include "time/Time.hpp"
#include <Eigen/Dense>
#include <functional>
#include <cstdint>

namespace xo {
  namespace kalman {
    /* encapsulate state (i.e. initial state,  and output after each step)
     * for a kalman filter
     */
    class KalmanFilterState {
    public:
      using utc_nanos = xo::time::utc_nanos;
      using VectorXd = Eigen::VectorXd;
      using MatrixXd = Eigen::MatrixXd;
      using uint32_t = std::uint32_t;

    public:
      KalmanFilterState();
      KalmanFilterState(uint32_t k,
			utc_nanos tk,
			VectorXd x,
			MatrixXd P);

      uint32_t step_no() const { return k_; }
      utc_nanos tm() const { return tk_; }
      /* with n = .n_state():
       *   x_ is [n x 1] vector
       *   P_ is [n x n] matrix,
       */
      uint32_t n_state() const { return x_.size(); }
      VectorXd const & state_v() const { return x_; }
      MatrixXd const & state_cov() const { return P_; }

    private:
      /* step# k,  advances by +1 on each filter step */
      uint32_t k_ = 0;
      /* time t(k) */
      utc_nanos tk_;
      /* [n x 1] (estimated) system state xk = x(k) */
      VectorXd x_;
      /* [n x n] covariance matrix for error assoc'd with with x(k)
       *   P(i,j) is the covariance of (ek[i], ek[j]),
       * where ex(k) is the difference (x(k) - x_(k))
       * between estimated state x(k)
       * (= this->x_) and model state x_(k)
       */
      MatrixXd P_;
    }; /*KalmanFilterState*/

    /* KalmanFilterStateExt:
     * adds additional details from filter step to KalmanFilterState
     */
    class KalmanFilterStateExt : public KalmanFilterState {
    public:
      using MatrixXd = Eigen::MatrixXd;
      using int32_t = std::int32_t;

    public:
      KalmanFilterStateExt(uint32_t k,
			   utc_nanos tk,
			   VectorXd x,
			   MatrixXd P,
			   MatrixXd K,
			   int32_t j);

      int32_t observable() const { return j_; }
      MatrixXd const & gain() const { return K_; }

    private:
      /* if -1:  not used;
       * if >= 0: identifies j'th of m observables;
       * gain .K applies just to information obtainable from
       * observing that scalar variable
       */
      int32_t j_ = -1;
      /* if .j is -1:
       *   [n x n] kalman gain
       * if .j >= 0:
       *   [n x 1] kalman gain for observable #j
       */
      MatrixXd K_;
    }; /*KalamnFilterStateExt*/
      
  } /*namespace filter*/
} /*namespace xo*/

/* end KalmanFilterState.hpp */
