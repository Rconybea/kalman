/* @file fixed.hpp */

#pragma once

#include <iostream>

namespace logutil {
  class fixed {
  public:
    fixed(double x, uint16_t prec) : x_{x}, prec_{prec} {}

    /* print this value */
    double x_;
    /* precision */
    uint16_t prec_ = 0;
  }; /*fixed*/

  inline std::ostream &
  operator<<(std::ostream & s, fixed const & fx)
  {
    std::streamsize orig_flags = s.flags();
    std::streamsize orig_p  = s.precision();

    s.flags(std::ios::fixed);
    s.precision(fx.prec_);
    s << fx.x_;

    s.flags(orig_flags);
    s.precision(orig_p);

    return s;
  } /*operator<<*/
} /*namespace logutil*/

/* end fixed.hpp */

