/* @file Time.hpp */

#pragma once

#include <chrono>
#include <cstdint>

namespace xo {
namespace time {

using utc_nanos = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::nanoseconds>;

using nanos = std::chrono::nanoseconds;
using hours = std::chrono::hours;
using days = std::chrono::days;

} /*namespace time*/
} /*namespace xo*/

/* end Time.hpp */
