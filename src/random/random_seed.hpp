/* @file random_seed.hpp */

#include "logutil/array.hpp"
#include <iostream>
#include <cstdint>
#include <stdlib.h>

namespace xo {
  namespace random {
    /* generate a 64-bit random seed using /dev/urandom or similar source.
     * This is relatively expensive;  at least cost of a system call
     * + may block if host has rebooted recently
     *
     * Require:
     * - T is null-constructible.
     *
     * return value will contain a T-instance in which representation
     * has been populated with random bits.   Expecting T to be something
     * like int32_t, or std::array<uint64_t, ..>
     */
    template<typename T>
    void random_seed(T * p_seed) {
      /* NOTE: arc4random_buf() works on darwin/nix;
       *       probably need to do something else on intel linux
       */
      arc4random_buf(p_seed, sizeof(*p_seed));
    } /*random_seed*/

    template<typename T>
    T random_seed() {
      T retval;
      random_seed(&retval);

      return retval;
    } /*random_seed*/

    /* RAII-style randome-number seed
     *
     * Usage:
     *
     *   Seed<uint64_t> seed;
     *   auto rgen = UnitIntervalGen<xoshiro256ss>::make(seed);
     */
    template<typename T>
    struct Seed {
      Seed() { random_seed(&seed_); }

      operator T const & () const { return seed_; }

      T seed_;
    }; /*Seed*/

    template<typename T>
    inline std::ostream &
    operator<<(std::ostream & os,
	       random::Seed<T> const & x)
    {
      using logutil::operator<<;

      os << x.seed_;
      return os;
    } /*operator<<*/

  } /*namespace random*/

} /*namespace xo*/

/* end random_seed.hpp */

   
