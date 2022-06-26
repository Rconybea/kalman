/* @file OptionId.hpp */

#pragma once

#include <iostream>
#include <cstdint>

namespace xo {
  namespace option {
    class OptionId {
    public:
      OptionId() = default;
      explicit OptionId(uint32_t num) : num_(num) {}

      static OptionId invalid() { return OptionId(-1); }

      static int32_t compare(OptionId x, OptionId y) {
	return x.num() - y.num();
      } /*compare*/

      uint32_t num() const { return num_; }

    private:
      /* unique id# for an option */
      uint32_t num_ = 0;
    }; /*OptionId*/

    inline std::ostream &
    operator<<(std::ostream & os, OptionId id) {
      os << id.num();
      return os;
    } /*operator<<*/
  } /*namespace option*/

} /*namespace xo*/

/* end OptionId.hpp */
