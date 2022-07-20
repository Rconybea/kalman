/* @file KalmanFilterInput.hpp */

#pragma once

#include "time/Time.hpp"
#include <Eigen/Dense>
#include <cstdint>

namespace xo {
  namespace kalman {
    class KalmanFilterInput {
    public:
      using VectorXd = Eigen::VectorXd;
      using utc_nanos = xo::time::utc_nanos;
      using uint32_t = std::uint32_t;

    public:
      KalmanFilterInput() = default;
      explicit KalmanFilterInput(utc_nanos tkp1, VectorXd z)
	: tkp1_(tkp1), z_{std::move(z)} {}

      utc_nanos tkp1() const { return tkp1_; }
      uint32_t n_obs() const { return z_.size(); }
      VectorXd const & z() const { return z_; }

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      /* t(k+1) - asof time for observations .z */
      utc_nanos tkp1_ = xo::time::Time::epoch();
      /* [m x 1] observation vector z(k) */
      VectorXd z_;
    }; /*KalmanFilterInput*/

    
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterInput.hpp */

