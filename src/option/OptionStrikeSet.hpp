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
      static StrikePair make_callput_pair(OptionId call_id,
					  OptionId put_id,
					  double strike,
					  utc_nanos expiry,
					  Pxtick pxtick);

      ref::brw<VanillaOption> call() const { return (*this)[0]; }
      ref::brw<VanillaOption>  put() const { return (*this)[1]; }

      ref::brw<VanillaOption> any_option(Callput prefer) const {
	if(prefer == Callput::call) {
	  if (this->call()) {
	    return this->call();
	  } else {
	    return this->put();
	  }
	} else {
	  if (this->put()) {
	    return this->put();
	  } else {
	    return this->call();
	  }
	}
      } /*any_option*/

      /* return #of options for this strike pair.
       * must be one of:
       * - 2 (call + put)
       * - 1 (call or put)
       * - 0 (empty strike pair)
       */
      std::size_t n_option() const {
	std::size_t n = 0;
	for(std::size_t i = 0; i<2; ++i) {
	  if ((*this)[i])
	    ++n;
	}
	return n;
      } /*n_option*/

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
					      OptionId start_id,
					      double lo_strike,
					      double  d_strike,
					      utc_nanos expiry,
					      Pxtick pxtick);

      std::size_t n_strike() const { return strike_v_.size(); }
      std::size_t n_option() const {
	std::size_t n = 0;
	auto fn = [&n](StrikePair const & k) { n += k.n_option(); };
	this->visit_strikes(fn);
	return n;
      } /*n_option*/
      
      /* call fn(p) for each option pair p in .strike_v[] */
      template<typename Fn>
      void visit_strikes(Fn fn) const {
	for(StrikePair const & strike_pair : this->strike_v_) {
	  fn(strike_pair);
	}
      } /*visit_strikes*/

      template<typename V>
      void append_strikes(V * v) const {
	for(StrikePair const & strike_pair : this->strike_v_) {
	  v->push_back(strike_pair);
	}
      } /*append_strikes*/

      template<typename V>
      void append_options(V * v) const {
	for(StrikePair const & strike_pair : this->strike_v_) {
	  if(strike_pair.call())
	    v->push_back(strike_pair.call().get());
	  if(strike_pair.put())
	    v->push_back(strike_pair.put().get());
	}
      } /*append_options*/

      void push_back(StrikePair const & x) { this->strike_v_.push_back(x); }

      bool verify_ok(bool may_throw) const;

      void display(std::ostream & os) const;
      std::string display_string() const;

    private:
      OptionStrikeSet() = default;

    private:
      /* call/put pairs, in increasing effective strike order */
      std::vector<StrikePair> strike_v_;
    }; /*OptionStrikeSet*/

    inline std::ostream &
    operator<<(std::ostream & os, ref::rp<OptionStrikeSet> const & oset) {
      oset->display(os);
      return os;
    } /*operator<<*/

  } /*namespace option*/
} /*namespace xo*/

/* end OptionStrikeSet.hpp */
