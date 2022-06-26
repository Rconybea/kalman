/* @file Size.hpp */

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
  namespace option {
    /* size (shares / contracts) with typesafety */
    class Size {
    public:
      Size() = default;

      static Size invalid() { return Size(sc_invalid_rep); }
      static Size from_int(uint32_t x) { return Size(x); }

      bool is_valid() const { return rep_ != sc_invalid_rep; }
      bool is_invalid() const { return rep_ == sc_invalid_rep; }
      uint32_t to_int() const { return rep_; }

    private:
      explicit Size(uint32_t rep) : rep_{rep} {}

      static constexpr uint32_t sc_invalid_rep = static_cast<uint32_t>(-1);

    private:
      uint32_t rep_ = sc_invalid_rep;
    }; /*Size*/

    inline std::ostream &
    operator<<(std::ostream & os, Size x) {
      os << x.to_int();
      return os;
    } /*operator<<*/
  } /*namespace option*/
} /*namespace xo*/
