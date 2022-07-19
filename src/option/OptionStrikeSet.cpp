/* @file OptionExpirySet.cpp */

/* Time.hpp: need to see logutil::operator<< before xtag header gets pulled in */
#include "time/Time.hpp"
#include "OptionStrikeSet.hpp"
#include "logutil/scope.hpp"

namespace xo {
  using xo::ref::rp;
  using logutil::xtag;
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
    StrikePair::make_callput_pair(OptionId call_id, OptionId put_id, 
				  double strike, utc_nanos expiry, Pxtick pxtick)
    {
      return StrikePair(VanillaOption::make(call_id, Callput::call, strike, expiry, pxtick),
			VanillaOption::make( put_id, Callput::put,  strike, expiry, pxtick));
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
			     OptionId start_id,
			     double lo_strike,
			     double d_strike,
			     utc_nanos expiry,
			     Pxtick pxtick)
    {
      rp<OptionStrikeSet> retval
	= OptionStrikeSet::empty();

      OptionId next_id = start_id;

      for(uint32_t i = 0; i < n; ++i) {
	double i_strike = lo_strike + (i * d_strike);

	OptionId call_id = next_id;
	OptionId  put_id(call_id.num() + 1);

	retval->push_back(StrikePair::make_callput_pair(call_id, put_id,
							i_strike, expiry, pxtick));

	next_id = OptionId(put_id.num() + 1);
      }

      return retval;
    } /*regular*/

    void
    OptionStrikeSet::display(std::ostream & os) const
    {
      OptionId lo_id;
      double lo_strike = 0.0;
      OptionId hi_id;
      double hi_strike = 0.0;

      if(!(this->strike_v_.empty())) {
	ref::brw<VanillaOption> lo_option
	  = this->strike_v_[0].any_option(Callput::call);

	if(lo_option) {
	  lo_id = lo_option->id();
	  lo_strike = lo_option->effective_strike();
	}

	ref::brw<VanillaOption> hi_option
	  = this->strike_v_[this->strike_v_.size() - 1].any_option(Callput::put);

	if(hi_option) {
	  hi_id = hi_option->id();
	  hi_strike = hi_option->effective_strike();
	}
      }

      os << "<OptionStrikeSet"
	 << xtag("n_strike", this->strike_v_.size())
	 << xtag("lo_id", lo_id)
	 << xtag("lo_strike", lo_strike)
	 << xtag("hi_id", hi_id)
	 << xtag("hi_strike", hi_strike)
	 << ">";
    } /*display*/

    std::string
    OptionStrikeSet::display_string() const
    {
      std::stringstream ss;
      this->display(ss);

      return ss.str();
    } /*display_string*/
  } /*namespace option*/
} /*namespace xo*/

/* @file OptionExpirySet.cpp */
