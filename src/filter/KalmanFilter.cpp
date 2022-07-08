/* @file KalmanFilter.cpp */

#include "KalmanFilter.hpp"
#include "Eigen/src/Core/Matrix.h"

namespace xo {
  using xo::time::utc_nanos;
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

    KalmanFilterState
    KalmanFilterEngine::extrapolate(utc_nanos tkp1,
				    KalmanFilterState const & s,
				    KalmanFilterTransition const & f)
    {
      /* prior estimates at t(k) */
      VectorXd const & x = s.state_v();
      MatrixXd const & P = s.state_cov();

      /* model change from t(k) -> t(k+1) */
      MatrixXd const & F = f.transition_mat();
      MatrixXd const & Q = f.transition_cov();

      /* x(k+1|k) */
      VectorXd x_ext = F * x;

      /* P(k+1|k) */
      MatrixXd P_ext = (F * P * F.transpose()) + Q;

      return KalmanFilterState(s.step_no() + 1,
			       tkp1,
			       x_ext,
			       P_ext);
    } /*extrapolate*/

    MatrixXd
    KalmanFilterEngine::kalman_gain(KalmanFilterState const & skp1_ext,
				    KalmanFilterObservable const & h)
    {
      /* P(k+1|k) */
      MatrixXd const & P_ext = skp1_ext.state_cov();

      MatrixXd const & H = h.observable();
      MatrixXd const & R = h.observable_cov();
      
      /* kalman gain:
       *                         T  -1
       *   K(k+1) = P(k+1|k).H(k) .M
       *
       *                         T /                  T        \ -1
       *          = P(k+1|k).H(k) .| H(k).P(k+1|k).H(k) + R(k) |
       *                           \                           /
       *
       *                           
       *
       * Notes:
       * 1. the matrix M being inverted is symmetric,  since represents covariances.
       * 2. if diagonal of R(k) has no zeroes (i.e. all measurements are subject to error),
       *    then it must be non-negative definite
       * 3. unless observation errors are perfectly correlated, M(k)
       *    is positive definite.
       * 4. even though 3. holds,  there may be a nearby non-positive-definite matrix M+dM.
       *    Factoring M with finite-precision arithmetic may run into difficulty if M
       *    is only 'slighlty' +ve definite.
       *    If necessary add small diagonal correction D to M,
       *    sufficient to make M+D positive definite.
       *    This is equivalent to introducing additional
       *    uncorrelated observation error,   so benign from a robustness perspective
       * 5. In generally we usually want to avoid fully realizing a matrix inverse.
       *    In this case need to explicitly compute K as ingredient used to
       *    correct state covariance later.
       * 6. However,  if R is diagonal (which is quite likely),   then it's easy
       *    to decompose a suite of vector observations z(k+1) = [z1, ..zm]T
       *    into separate zi, with dt=0 separating them.
       *    Can use this to avoid computing the inverse.
       */

      MatrixXd M = H * P_ext * H.transpose() + R;

      /* will use to write M as:
       *
       *        T      T
       *   M = P .L.D.L .P
       *
       * where:
       *   P is a permutation matrix
       *   L is lower triangular,  with unit diagonal
       *   D is diagonal
       */
      LDLT<MatrixXd> ldlt = M.ldlt();

      /* solve for the identity matrix to realize the inverse this way */
      MatrixXd I = MatrixXd::Identity(M.rows(), M.cols());

      /*  -1
       * M
       */
      MatrixXd M_inv = ldlt.solve(I);

      /* K(k+1) */
      MatrixXd K = P_ext * H * M_inv;

      return K;
    } /*kalman_gain*/

    KalmanFilterStateExt
    KalmanFilterEngine::correct(KalmanFilterState const & skp1_ext,
				KalmanFilterObservable const & h,
				KalmanFilterInput const & zkp1)
    {
      uint32_t n = skp1_ext.n_state();
      MatrixXd K = kalman_gain(skp1_ext, h);
      MatrixXd const & H = h.observable();
      VectorXd const & z = zkp1.z();
      VectorXd const & x_ext = skp1_ext.state_v();
      MatrixXd const & P_ext = skp1_ext.state_cov();

      /* innov: difference between 'actual observations'
       * and 'predicted observations'
       */
      VectorXd innov = z - H * x_ext;

      VectorXd xkp1 = x_ext + K * innov;
      MatrixXd I = MatrixXd::Identity(n, n);
      MatrixXd Pkp1 = (I - K * H) * P_ext;

      return KalmanFilterStateExt(skp1_ext.step_no(),
				  skp1_ext.tm(),
				  xkp1,
				  Pkp1,
				  K);
    } /*correct*/

    KalmanFilterStateExt
    KalmanFilterEngine::step(utc_nanos tkp1,
			     KalmanFilterState const & sk,
			     KalmanFilterTransition const & Fk,
			     KalmanFilterObservable const & Hkp1,
			     KalmanFilterInput const & zkp1)
    {
      KalmanFilterState skp1_ext
	= KalmanFilterEngine::extrapolate(tkp1, sk, Fk);

      KalmanFilterStateExt skp1
	= KalmanFilterEngine::correct(skp1_ext, Hkp1, zkp1);

      return skp1;
    } /*step*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilter.cpp */
