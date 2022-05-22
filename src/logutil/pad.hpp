/* @file pad.hpp */

#pragma once

#include <iostream>

namespace logutil {
  class pad_impl {
  public:
    pad_impl(int32_t n) : n_pad_(n) {}

    uint32_t n_pad() const { return n_pad_; }

  private:
    uint32_t n_pad_ = 0;
  }; /*pad_impl*/

  inline pad_impl
  pad(uint32_t n) { return pad_impl(n); }

  inline std::ostream &
  operator<<(std::ostream &s,
	     pad_impl const &pad)
  {
    for(uint32_t i=0; i<pad.n_pad(); ++i)
      s << " ";
    return s;
  } /*operator<<*/

} /*namespace logutil*/

/* end pad.hpp */
