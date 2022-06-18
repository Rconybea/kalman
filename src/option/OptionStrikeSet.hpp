/* @file OptionExpirySet.hpp */

#pragma once

#include "option/VanillaOption.hpp"
#include <vector>
#include <array>

namespace xo {
  namespace option {
    /* A pair of options with the same {underlying, expiry, strike} price
     *
     * NOTE: when dealing with US listed options,  on rare occasions
     *       it's possible that pair is broken,  with one option missing
     */
    class StrikePair : std::array<ref::rp<VanillaOption>, 2> {
    public:
      using utc_nanos = time::utc_nanos;

    public:
      StrikePair(ref::rp<VanillaOption> const & call,
		 ref::rp<VanillaOption> const &  put)
      {
	(*this)[0] = call;
	(*this)[1] = put;
      } /*ctor*/

      /* create call/put pair for a particular strike and expiry
       */
      static StrikePair make_callput_pair(double strike, utc_nanos expiry, Pxtick pxtick);

      ref::brw<VanillaOption> call() const { return (*this)[0]; }
      ref::brw<VanillaOption>  put() const { return (*this)[1]; }

      /* if true,  may throw exception when verification fails.
       * if false,  will return success/fail without throwing.
       */
      bool verify_ok(bool may_throw) const;
    }; /*StrikePair*/
    
    /* A set of options that share common attributes:
     * - underlying
     * - expiry
     * - exercise style (american | european)
     *
     * options are organized in call/put pairs,
     * by increasing effective strike
     */
    class OptionStrikeSet : public ref::Refcount {
    public:
      using utc_nanos = time::utc_nanos;
      
    public:
      static ref::rp<OptionStrikeSet> empty() { return new OptionStrikeSet(); }

      /* build set with full pair per strike,
       * with n strikes lo_strike + i * d_strike,   i in [0 .. n-1]
       */
      static ref::rp<OptionStrikeSet> regular(std::uint32_t n,
					      double lo_strike,
					      double  d_strike,
					      utc_nanos expiry,
					      Pxtick pxtick);

      void push_back(StrikePair const & x) { this->strike_v_.push_back(x); }

      bool verify_ok(bool may_throw) const;

      /* call fn(p) for each pair p in .strike_v[] */
      template<typename Fn>
      void visit_strikes(Fn fn) const {
	for(StrikePair const & strike_pair : this->strike_v_) {
	  fn(strike_pair);
	}
      } /*visit_strikes*/

    private:
      OptionStrikeSet() = default;

    private:
      /* call/put pairs, in increasing effective strike order */
      std::vector<StrikePair> strike_v_;
    }; /*OptionStrikeSet*/
  } /*namespace option*/
} /*namespace xo*/

/* end OptionStrikeSet.hpp */
