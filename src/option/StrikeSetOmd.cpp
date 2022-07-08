/* @file StrikeSetOmd.cpp */

#include "StrikeSetOmd.hpp"

namespace xo {
  using logutil::xtag;

  namespace option {
    void
    Omd::notify_bbo(BboTick const & bbo_tick)
    {
      for (Side s : SideIter()) {
	if (bbo_tick.is_side_present(s)) {
	  this->tm_v_[side2int(s)] = bbo_tick.tm();
	  this->bbo_pxz2_.assign_pxz(s, bbo_tick.pxz2());
	}
      } 
    } /*notify_bbo*/

    // ----- OmdPair -----

    void
    OmdPair::notify_bbo(BboTick const & bbo_tick)
    {
      Omd & omd = (*this)[bbo_tick.id().strike_pair_ix()];

      omd.notify_bbo(bbo_tick);
    } /*notify_bbo*/

    // ----- StrikeSetOmd -----

    ref::rp<StrikeSetOmd>
    StrikeSetOmd::make(ref::rp<OptionStrikeSet> const & oset)
    {
      return new StrikeSetOmd(oset);
    } /*make*/

    StrikeSetOmd::StrikeSetOmd(ref::rp<OptionStrikeSet> const & oset)
      : option_set_{oset}
    {
      if (!oset)
	throw std::runtime_error("StrikeSetOmd:: expected non-null oset");

      this->omd_v_.resize(oset->n_strike());
    } /*ctor*/      

    Omd &
    StrikeSetOmd::lookup(OptionId id)
    {
      uint32_t k = id.strike_ix();

      if(k >= this->omd_v_.size())
        throw std::runtime_error(tostr("expected strike index for incoming tick"
				       " in range [0,k-1]",
				       xtag("k", k)));

      OmdPair & omd_pair = this->omd_v_[k];

      Omd & omd = omd_pair[id.strike_pair_ix()];

      return omd;
    } /*lookup*/

    Omd const &
    StrikeSetOmd::lookup(OptionId id) const
    {
      /* cast to use non-const version,
       * since non-constness only in return type
       */
      StrikeSetOmd * self = const_cast<StrikeSetOmd *>(this);

      return self->lookup(id);
    } /*lookup*/

    void
    StrikeSetOmd::notify_bbo(BboTick const & bbo_tick)
    {
      /* option id# is also index into .omd_v[] */
      if(bbo_tick.id().is_invalid())
	throw std::runtime_error("expected bbo tick with valid option id#");

      uint32_t k = bbo_tick.id().strike_ix();

      if(k >= this->omd_v_.size())
        throw std::runtime_error(tostr("expected strike index for incoming tick"
				       " in range [0,k-1]",
				       xtag("k", k)));

      OmdPair & omd_pair = this->omd_v_[k];

      omd_pair.notify_bbo(bbo_tick);
    } /*notify_bbo*/
  } /*namespace option*/
} /*namespace xo*/

/* end StrikeSetOmd.cpp */
