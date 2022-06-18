/* @file Pxtick.hpp */

#pragma once

#include "option/Price.hpp"
#include <cmath>
#include <cassert>

namespace xo {
  namespace option {
    /* all_penny : tick size is 0.01
     * penny_nickel : tick size is 0.01 below $3, 0.05 above $3
     * nickel_dime: tick size is 0.05 below $3, 0.10 above $3
     */
    enum class Pxtick { all_penny, penny_nickel, nickel_dime };

    struct PxtickUtil {
      static Price glb_tickmult(double ticksize, double x) {
	return Price::from_double(ticksize * std::floor(x / ticksize));
      } /*glb_tickmult*/

      static Price lub_tickmult(double ticksize, double x) {
	return Price::from_double(ticksize * std::ceil(x / ticksize));
      } /*lub_tickmult*/

      static Price round_tick(int32_t rounding_dirn, double ticksize, double x) {
	if(rounding_dirn == +1) {
	  return lub_tickmult(ticksize, x);
	} else if(rounding_dirn == -1) {
	  return glb_tickmult(ticksize, x);
	}

	assert(false);
	return Price();
      } /*round_tick*/

      /* dirn = +1:
       *   returns lowest price that is >= x (least upper bound),
       *   and is displayable according to pxtick
       * dirn = -1:
       *   returns highest price that is <= x (greatest lower bound),
       *   and displayable according to pxtick
       */
      static Price tickmult_aux(int dirn, Pxtick pxtick, double x) {
	switch(pxtick) {
	case Pxtick::all_penny:
	  return round_tick(dirn, 0.01, x);
	  break;
	case Pxtick::penny_nickel:
	  if(x > 3.0) {
	    /* 0.05 ticksize */
	    return round_tick(dirn, 0.05, x);
	  } else {
	    /* 0.01 ticksize */
	    return round_tick(dirn, 0.01, x);
	  }
	  break;
	case Pxtick::nickel_dime:
	  if(x > 3.0) {
	    /* 0.10 ticksize */
	    return round_tick(dirn, 0.10, x);
	  } else {
	    /* 0.05 ticksize */
	    return round_tick(dirn, 0.05, x);
	  }
	  break;
	} /*switch*/
      } /*tickmult_aux*/

      static Price glb_tick(Pxtick pxtick, double x) {
	return tickmult_aux(-1, pxtick, x);
      } /*glb_tick*/

      static Price lub_tick(Pxtick pxtick, double x) {
	return tickmult_aux(+1, pxtick, x);
      } /*lub_tick*/
    }; /*PxtickUtil*/

  } /*namespace option*/
} /*namespace xo*/

/* end Pxtick.hpp */
