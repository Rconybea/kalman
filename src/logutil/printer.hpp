/* @file printer.hpp */

#pragma once

#include <utility>

namespace xo {
  namespace print {
    /* print an event to a logfile
     * intended to be usable as EventSink argument
     * to RealizationSimSource<T, EventSink>
     */
    template<typename T, typename Stream>
    class printer {
    public:
      printer(Stream && os) : os_{std::move(os)} {}

      void operator()(T const & x) {
	this->os_ << x;
      }

    private:
      Stream os_;
    }; /*printer*/
  } /*namespace print*/
} /*namespace xo*/

/* end printer.hpp */
