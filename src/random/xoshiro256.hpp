/* @file xoshiro256.hpp */

#pragma once

#include <array>
#include <limits>
#include <cstdint>

namespace xo {
  namespace random {

    /* engine for producing 64-bit random numbers
     *
     * see https:/en.wikipedia.org/wiki/Xorshift#xoshiro256**
     */
    class xoshiro256ss {
    public:
      using result_type = uint64_t;
      using seed_type = std::array<uint64_t, 4>;

    public:
      xoshiro256ss(seed_type const & seed)
	: s_(seed)
      {}

      /* fallback version */
      xoshiro256ss(uint64_t seed)
      {
	this->s_[0] = 0;
	this->s_[1] = seed;
	this->s_[2] = 0;
	this->s_[3] = 0;

	generate();
      }

      static constexpr uint64_t min() { return 0; }
      static constexpr uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

      static uint64_t rol64(uint64_t x, int64_t k)
      {
	return (x << k) | (x >> (64 - k));
      }

      uint64_t generate()
      {
	std::array<uint64_t, 4> & s = (this->s_);
	uint64_t const result = rol64(s[1] * 5, 7) * 9;
	uint64_t const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rol64(s[3], 45);

	return result;
      } /*generate*/

      uint64_t operator()() { return generate(); }

    private:
      /* state */
      std::array<uint64_t, 4> s_;
    }; /*xoshiro256ss*/
  } /*namespace random*/
} /*namespace xo*/

/* end xoshiro256.hpp */
