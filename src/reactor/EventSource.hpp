/* @file EventSource.hpp */

#pragma once

#include "reactor/ReactorSource.hpp"

namespace xo {
  namespace reactor {
    template</*typename Event,*/
	     typename Callback
             /*void (Callback::*member_fn)(Event const &)*/> 
    class EventSource : public ReactorSource {
    public:
      virtual void add_callback(ref::rp<Callback> const & cb) = 0;
      virtual void remove_callback(ref::rp<Callback> const & cb) = 0;
    }; /*EventSource*/
      
  } /*namespace reactor*/
} /*namespace xo*/

/* end EventSource.hpp */
