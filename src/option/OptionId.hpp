/* @file OptionId.hpp */

#pragma once

#include <cstdint>

namespace xo {
  namespace option {
    class OptionId {
    public:
      OptionId() = default;
      explicit OptionId(uint32_t num) : num_(num) {}

      static OptionId invalid() { return OptionId(-1); }

      uint32_t num() const { return num_; }

    private:
      /* unique id# for an option */
      uint32_t num_ = 0;
    }; /*OptionId*/
  } /*namespace option*/
} /*namespace xo*/

/* end OptionId.hpp */
