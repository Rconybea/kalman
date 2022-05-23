/* @file xoshiro256.hpp */

#pragma once

#include <limits>
#include <cstdint>

namespace xo {
  namespace random {

    /* engine for producing 64-bit random numbers
     *
     * see https:/en.wikipedia.org/wiki/Xorshift#xoshiro256**
     */
    class xoshiro256 {
    public:
      using result_type = uint64_t;

    public:
      xoshiro256(uint64_t seed)
      {
	this->s_[0] = 0;
	this->s_[1] = seed;
	this->s_[2] = 0;
	this->s_[3] = 0;
      }

      static constexpr uint64_t min() { return 0; }
      static constexpr uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

      static uint64_t rol64(uint64_t x, int64_t k)
      {
	return (x << k) | (x >> (64 - k));
      }

      uint64_t operator()()
      {
	uint64_t * s = this->s_;
	uint64_t const result = rol64(s[1] * 5, 7) * 9;
	uint64_t const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rol64(s[3], 45);

	return result;
      } /*xoshiro256ss*/

    private:
      /* state */
      uint64_t s_[4];
    }; /*xoshiro256*/
  } /*namespace random*/
} /*namespace xo*/

/* end xoshiro256.hpp */
