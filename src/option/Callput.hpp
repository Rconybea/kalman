/* @file Callput.hpp */

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
  } /*namespace option*/
} /*namespace xo*/

/* end Callput.hpp */
