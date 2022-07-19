/* @file OptionId.hpp */

#pragma once

#include "logutil/scope.hpp"
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

      bool is_valid() const { return num_ != static_cast<uint32_t>(-1); }
      bool is_invalid() const { return num_ == static_cast<uint32_t>(-1); }
      uint32_t num() const { return num_; }

      /* option id#'s are always generated in pairs;
       * call id#'s are always even
       * put id#'s are always odd
       */
      uint32_t strike_ix() const { return num_ / 2; }

      /* index into a {call, put} pair:
       * - 0 for even index values (call options)
       * - 1 for odd  index values (put  options)
       */
      uint32_t strike_pair_ix() const { return num_ % 2; }

      std::string display_string() const {
	return logutil::tostr("<OptionId :num ", num_, ">");
      } /*display_string*/

    private:
      /* unique id# for an option */
      uint32_t num_ = -1;
    }; /*OptionId*/

    inline bool operator==(OptionId x, OptionId y) { return x.num() == y.num(); }
    inline bool operator!=(OptionId x, OptionId y) { return x.num() != y.num(); }
    inline bool operator< (OptionId x, OptionId y) { return x.num() <  y.num(); }
    inline bool operator<=(OptionId x, OptionId y) { return x.num() <= y.num(); }
    inline bool operator>=(OptionId x, OptionId y) { return x.num() >= y.num(); }
    inline bool operator> (OptionId x, OptionId y) { return x.num() >  y.num(); }

    inline std::ostream &
    operator<<(std::ostream & os, OptionId id) {
      os << id.num();
      return os;
    } /*operator<<*/
  } /*namespace option*/

} /*namespace xo*/

/* end OptionId.hpp */
