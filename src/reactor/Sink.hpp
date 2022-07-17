/* @file Sink.hpp */

#pragma once

#include "reactor/Source.hpp"
#include "reflect/demangle.hpp"
#include "logutil/tag.hpp"

namespace xo {
  namespace reactor {
    class AbstractSink : public virtual ref::Refcount {
    public:
      virtual ~AbstractSink() = default;

      /* attach an input source.
       * typically this means calling src.add_callback()
       * with a function thats calls a .notify_xxx() method
       * on this Sink
       */
      virtual void attach_source(ref::rp<AbstractSource> const & src) = 0;
    }; /*AbstractSink*/

    /* Sink for events of type T */
    template<typename T>
    class Sink1 : public AbstractSink {
    public:
      /* convenience:  convert abstract sink to Sink1<T>*,
       * or throw
       */
      static Sink1<T> * require_native(std::string_view caller,
				       ref::rp<AbstractSink> const & sink)
      {
	using logutil::xtag;

	Sink1<T> * native_sink = dynamic_cast<Sink1<T> *>(sink.get());

	if (!native_sink) {
	  throw std::runtime_error(tostr(std::string(caller),
					 ": expected sink accepting type",
					 xtag("T", reflect::type_name<T>())));
	}

	return native_sink;
      } /*require_native*/

      /* notify on incoming event */
      virtual void notify_ev(T const & ev) = 0;

      /* invoke these when this sink added to, or removed from, a source */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}

      // ----- inherited from AbstractSink -----

      virtual void attach_source(ref::rp<AbstractSource> const & src) override {
	src->attach_sink(this);
      } /*attach_source*/

    }; /*Sink1*/
    
    template<typename T, typename Fn>
    class SinkToFunction : public Sink1<T> {
    public:
      SinkToFunction(Fn fn) : fn_{std::move(fn)} {}

      virtual void notify_ev(T const & ev) override {
	fn_(ev);
      } /*notify_ev*/

    private:
      Fn fn_;
    }; /*SinkToFunction*/

  } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.hpp */
