/* @file Callput.hpp */

#pragma once

namespace xo {
  namespace option {
    enum class Callput { call, put };

    struct CallputUtil {
      static inline Callput other(Callput x) {
	switch(x) {
	case Callput::call:
	  return Callput::put;
	case Callput::put:
	  return Callput::call;
	}
      } /*other*/
    }; /*CallputUtil*/

    inline std::ostream & operator<<(std::ostream & os, Callput x) {
      switch(x) {
      case Callput::call:
	os << "call";
	break;
      case Callput::put:
	os << "put";
	break;
      default:
	os << "???";
      }

      return os;
    } /*operator<<*/

  } /*namespace option*/
} /*namespace xo*/

/* end Callput.hpp */
