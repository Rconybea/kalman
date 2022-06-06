/* @file array.hpp */

#pragma once

#include <iostream>
#include <array>

namespace logutil {
  template<typename T, size_t N>
  inline std::ostream &
  operator<<(std::ostream & os,
	     std::array<T, N> const & v)
  {
    os << "[";
    for(size_t i = 0; i < N; ++i) {
      if(i > 0)
	os << " ";
      os << v[i];
    }
    os << "]";
    return os;
  } /*operator<<*/
} /*namespace logutil*/

/* end array.hpp */
