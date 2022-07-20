/* @file print_eigen.hpp */

#include <Eigen/Dense>
#include <cstdint>

namespace logutil {
  template<typename T>
  class matrix {
  public:
    matrix(T x) : x_{std::move(x)} {}

    /* print this value */
    T x_;
  }; /*matrix*/

  template<typename T>
  using vector = matrix<T>;

  template<typename T>
  inline std::ostream &
  operator<<(std::ostream & s, matrix<T> const & mat)
  {
    s << "[";
    for(std::uint32_t i = 0, m = mat.x_.rows(); i<m; ++i) {
      if(i > 0)
	s << "; ";

      for(std::uint32_t j = 0, n = mat.x_.cols(); j<n; ++j) {
	if(j > 0)
	  s << ' ';

	s << mat.x_(i, j);
      }
    }
    s << "]";
    
    return s;
  } /*operator<<*/
} /*namespace logutil*/

/* end print_eigen.hpp */
