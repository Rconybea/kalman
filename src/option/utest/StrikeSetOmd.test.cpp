/* @file StrikeSetOmd.cpp */

#include "option/StrikeSetOmd.hpp"
//#include "option_util/Pxtick.hpp"
#include <catch2/catch.hpp>

namespace xo {
  using xo::option::StrikeSetOmd;
  using xo::option::Omd;
  using xo::option::OptionStrikeSet;
  using xo::option::BboTick;
  using xo::option::PxSize2;
  using xo::option::Side;
  using xo::option::Price;
  using xo::option::Size;
  using xo::option::Pxtick;
  using xo::option::OptionId;
  using xo::time::Time;
  using xo::time::utc_nanos;
  using xo::ref::rp;

  namespace ut {
    TEST_CASE("strikeset-omd-empty", "[strikeset-omd]") {
      rp<OptionStrikeSet> empty_ss = OptionStrikeSet::empty();

      REQUIRE(empty_ss.get() != nullptr);
      
      rp<StrikeSetOmd> omd = StrikeSetOmd::make(empty_ss);

      REQUIRE(omd.get() != nullptr);

      /* empty omd will complain if try to send it a tick */
      {
        utc_nanos t0 = Time::ymd_hms_usec(20220617 /*ymd*/,
					  173905 /*hms*/,
                                          123456 /*usec*/);

        BboTick sample_tk(t0, OptionId(0),
                          PxSize2(Size::from_int(1),
				  Price::from_double(0.1),
                                  Price::from_double(0.2),
				  Size::from_int(2)));

        bool ex_flag = false;

        try {
          /* will throw exception */
          omd->notify_bbo(sample_tk);
        } catch (std::exception &ex) {
          ex_flag = true;
        }

        REQUIRE(ex_flag == true);
      }
    } /*TEST_CASE(strikeset-omd-empty)*/

    TEST_CASE("strikeset-omd-1strike", "[strikeset-omd]") {
      utc_nanos expiry_tm = Time::ymd_hms_usec(20220721 /*ymd*/,
					       173000 /*hms*/,
					       0 /*usec*/);

      rp<OptionStrikeSet> ss
	= OptionStrikeSet::regular(1 /*n-strike*/,
				   OptionId(0) /*start-id*/,
				   10.0 /*lo-strike*/,
				   1.0 /*d-strike*/,
				   expiry_tm,
				   Pxtick::penny_nickel /*pxtick*/);

      REQUIRE(ss->n_strike() == 1);
      REQUIRE(ss->n_option() == 2);
      REQUIRE(ss.get() != nullptr);

      rp<StrikeSetOmd> ss_omd = StrikeSetOmd::make(ss);

      OptionId id0 = OptionId(0);
      Omd & omd = ss_omd->lookup(id0);

      /* no ticks */
      REQUIRE(omd.is_bid_present() == false);
      REQUIRE(omd.is_ask_present() == false);

      /* can send omd ticks for options with id in {0, 1} */
      utc_nanos t0 = Time::ymd_hms_usec(20220705 /*ymd*/,
					133000 /*hms*/,
					123456 /*usec*/);

      BboTick sample_tk0(t0,
			 id0,
			 PxSize2(Size::from_int(2),
				 Price::from_double(0.21),
				 Price::from_double(0.22),
				 Size::from_int(5)));

      /* will not throw exception */
      ss_omd->notify_bbo(sample_tk0);

      CHECK(omd.is_bid_present() == true);
      CHECK(omd.is_ask_present() == true);
      CHECK(omd.tm(Side::bid) == sample_tk0.tm());
      CHECK(omd.tm(Side::ask) == sample_tk0.tm());
      CHECK(omd.size(Side::bid) == sample_tk0.pxz2().size(Side::bid));
      CHECK(omd.size(Side::ask) == sample_tk0.pxz2().size(Side::ask));
      CHECK(omd.px(Side::bid) == sample_tk0.pxz2().px(Side::bid));
      CHECK(omd.px(Side::ask) == sample_tk0.pxz2().px(Side::ask));
    } /*TEST_CASE(strike-omd-1strike)*/
  } /*namespace ut*/
} /*namespace xo*/

/* end StrikeSetOmd.cpp */
