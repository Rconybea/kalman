/* @file junk.cpp */

#include "KalmanConfig.h"

#include <iostream>
#include <Eigen/Dense>

int main(int argc, char **argv) {
  using Eigen::MatrixXd;
  using std::cout;
  using std::endl;

  cout << "Hello world" << endl;

  MatrixXd m(2,2);

  m(0,0) = 3;
  m(1,0) = 2.5;
  m(0,1) = -1;
  m(1,1) = m(1,0) + m(0,1);

  std::cout << m << std::endl;

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

  
