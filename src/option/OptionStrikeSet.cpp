/* @file OptionExpirySet.cpp */

/* Time.hpp: need to see logutil::operator<< before xtag header gets pulled in */
#include "time/Time.hpp"
#include "OptionStrikeSet.hpp"
#include "logutil/scope.hpp"

namespace xo {
  using xo::ref::rp;
  //using logutil::operator<<;

  namespace option {
    bool
    StrikePair::verify_ok(bool may_throw) const
    {
      using logutil::tostr;
      using logutil::xtag;

      ref::brw<VanillaOption> call_option = this->call();
      ref::brw<VanillaOption>  put_option = this->put();

      if(call_option && (call_option->callput() != Callput::call)) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:"
					 " expected call option in slot 0"));
	}
	return false;
      }

      if(put_option && (put_option->callput() != Callput::put)) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:"
					 " expected put option in slot 1"));
	}
	return false;
      }

      if(!call_option || !put_option)
	return true;

      /* both {call, put} options present */

      if(call_option->pxmult() != put_option->pxmult()) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:"
					 " options with pxmult m1,m2 found where"
					 " equal multipliers expected",
					 xtag("m1", call_option->pxmult()),
					 xtag("m2",  put_option->pxmult())));
	}
	return false;
      }

      if(call_option->delivmult() != put_option->delivmult()) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:"
					 " options with delivmult d1,d2 found where"
					 " equal multipliers expected",
					 xtag("d1", call_option->delivmult()),
					 xtag("d2",  put_option->delivmult())));
	}
	return false;
      }

      if(call_option->stated_strike() != put_option->stated_strike()) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:",
					 " options with stated strike k1,k2 found where"
					 " equal strikes expected",
					 xtag("k1", call_option->stated_strike()),
					 xtag("k2",  put_option->stated_strike())));
	}
	return false;
      }

      if(call_option->expiry() != put_option->expiry()) {
	if(may_throw) {
	  throw std::runtime_error(tostr("StrikePair::verify_ok:"
					 " options with expiries x1,x2 found where"
					 " equal values expected",
					 xtag("x1", call_option->expiry()),
					 xtag("x2",  put_option->expiry())));
	}
	return false;
      }
	
      return true;
    } /*verify_ok*/

    StrikePair
    StrikePair::make_callput_pair(double strike, utc_nanos expiry)
    {
      return StrikePair(VanillaOption::make(Callput::call, strike, expiry),
			VanillaOption::make(Callput::put,  strike, expiry));
    } /*make_callput_pair*/

    // ----- OptionStrikeSet -----

    bool
    OptionStrikeSet::verify_ok(bool may_throw) const
    {
      for(StrikePair const & ix : this->strike_v_) {
	if(!(ix.verify_ok(may_throw)))
	  return false;
      }

      return true;
    } /*verify_ok*/

    rp<OptionStrikeSet>
    OptionStrikeSet::regular(uint32_t n,
			     double lo_strike,
			     double d_strike,
			     utc_nanos expiry)
    {
      rp<OptionStrikeSet> retval
	= OptionStrikeSet::empty();

      for(uint32_t i = 0; i < n; ++i) {
	double i_strike = lo_strike + (i * d_strike);

	retval->push_back(StrikePair::make_callput_pair(i_strike, expiry));
      }

      return retval;
    } /*regular*/
  } /*namespace option*/
} /*namespace xo*/

/* @file OptionExpirySet.cpp */
