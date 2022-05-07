/* @file junk.cpp */

#include "KalmanConfig.h"
#include "logutil/scope.hpp"
#include <Eigen/Dense>
#include <iostream>

int
main(int argc, char **argv)
{
  // see:
  //   https: // eigen.tuxfamily.org/dox/GettingStarted.html

  using Eigen::MatrixXd;
  using Eigen::VectorXd;
  using std::cout;
  using std::endl;

  cout << "Hello world" << endl;

  MatrixXd m(2, 2);
  MatrixXd c = MatrixXd::Constant(2, 2, 1.1);
  VectorXd v(2);

  m(0,0) = 3;
  m(1,0) = 2.5;
  m(0,1) = -1;
  m(1,1) = m(1, 0) + m(0, 1);

  cout << "m=" << m << endl;

  v(0) = 1;
  v(1) = -1;

  cout << "v=" << v << endl;

  cout << "c=" << c << endl;

  cout << "m*c=" << m*c << endl;

  cout << "m*c*v=" << m*c*v << endl;

  if (argc < 2) {
    // report version
    cout << argv[0]
	 << " version "
	 << Kalman_VERSION_MAJOR
	 << "."
	 << Kalman_VERSION_MINOR
	 << endl;

    cout << "usage: " << argv[0] << " ..." << endl;

    return 1;
  }
}

/* junk.cpp */

  
