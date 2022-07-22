/* @file KalmanFilterObservable.hpp */

#pragma once

#include "time/Time.hpp"
#include <Eigen/Dense>
#include <cstdint>

namespace xo {
  namespace kalman {
    class KalmanFilterObservable {
    public:
      using MatrixXd = Eigen::MatrixXd;
      
    public:
      KalmanFilterObservable() = default;
      KalmanFilterObservable(MatrixXd H, MatrixXd R)
	: H_{std::move(H)}, R_{std::move(R)} {
	assert(this->check_ok());
      } /*ctor*/

      uint32_t n_state() const { return H_.cols(); }
      uint32_t n_observable() const { return H_.rows(); }
      MatrixXd const & observable() const { return H_; }
      MatrixXd const & observable_cov() const { return R_; }
      
      bool check_ok() const {
	uint32_t m = H_.rows();
	bool r_is_mxm = ((R_.cols() == m) && (R_.rows() == m));

	/* also would like to require: R is +ve definite */
	
	return r_is_mxm;
      } /*check_ok*/

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      /* [m x n] observation matrix */
      MatrixXd H_;
      /* [m x m] covariance matrix for observation noise */
      MatrixXd R_;
    }; /*KalmanFilterObservable*/

    inline std::ostream &
    operator<<(std::ostream & os, KalmanFilterObservable const & x)
    {
      x.display(os);
      return os;
    } /*operator<<*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterObservable.hpp */
   
