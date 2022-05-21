/* LogNormalProcess.hpp */

#pragma once

#include "process/BrownianMotion.hpp"
#include "process/ExpProcess.hpp"

namespace xo {
namespace process {

/* log-normal process -- i.e. logs follow brownian motion
 */
class LogNormalProcess {
public:
  static ExpProcess *make(utc_nanos t0, double sdev, uint64_t seed) {
    return ExpProcess::make(BrownianMotion::make(t0, sdev, seed));
  } /*make*/
};  /*LogNormalProcess*/

} /*namespace process*/
} /*namespace xo*/

/* end LogNormalProcess.hpp */
