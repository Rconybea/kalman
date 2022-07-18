/* @file Sink.hpp */

#pragma once

#include "reactor/Source.hpp"
#include "time/Time.hpp"
#include "reflect/demangle.hpp"
#include "logutil/tag.hpp"
#include <typeinfo>

namespace xo {
  namespace reactor {
    class AbstractSink : public virtual ref::Refcount {
    public:
      virtual ~AbstractSink() = default;

      /* identify datatype for items expected by this sink */
      virtual std::string_view item_type() const = 0;
      virtual std::type_info const * item_typeinfo() const = 0;

      virtual std::string_view self_typename() const = 0;
      virtual std::string_view parent_typename() const = 0;
      virtual std::type_info const * parent_typeinfo() const = 0;

      /* attach an input source.
       * typically this means calling src.add_callback()
       * with a function thats calls a .notify_xxx() method
       * on this Sink
       */
      virtual void attach_source(ref::rp<AbstractSource> const & src) = 0;
    }; /*AbstractSink*/

    /* Sink for events of type T
     *
     * inheritance:
     *   ref::Refcount
     *     ^
     *     isa
     *     |
     *   reactor::AbstractSink
     *     ^
     *     isa
     *     |
     *   reactor::Sink1<T>
     */
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
	  std::type_info const * sink_parent_typeinfo
	    = sink->parent_typeinfo();

	  std::size_t src_hashcode = typeid(T).hash_code();

	  throw std::runtime_error(tostr(std::string(caller),
					 ": wanted to sink S,  but sink expects T",
					 xtag("T", sink->item_type()),
					 xtag("S", reflect::type_name<T>()),
					 xtag("required_hashcode", typeid(Sink1<T>).hash_code()),
					 xtag("required_name", typeid(Sink1<T>).name()),
					 xtag("src_hashcode", src_hashcode),
					 xtag("sink_hashcode", sink->item_typeinfo()->hash_code()),
					 xtag("sink_parent_hashcode", sink_parent_typeinfo->hash_code()),
					 xtag("sink_parent_name", sink_parent_typeinfo->name()),
					 xtag("sink.type", sink->self_typename()),
					 xtag("sink.parent_type", sink->parent_typename())));
	}

	return native_sink;
      } /*require_native*/

      /* notify on incoming event */
      virtual void notify_ev(T const & ev) = 0;

      /* invoke these when this sink added to, or removed from, a source */
      virtual void notify_add_callback() {}
      virtual void notify_remove_callback() {}

      // ----- inherited from AbstractSink -----

      virtual std::string_view item_type() const override { return reflect::type_name<T>(); }
      virtual std::type_info const * item_typeinfo() const override { return &typeid(T); }
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

      virtual std::string_view self_typename() const override { return reflect::type_name<SinkToFunction>(); }
      virtual std::string_view parent_typename() const override { return reflect::type_name<Sink1<T>>(); }
      virtual std::type_info const * parent_typeinfo() const override { return &typeid(Sink1<T>); }

    private:
      Fn fn_;
    }; /*SinkToFunction*/

    /* sink that prints to console */
    template<typename T>
    class SinkToConsole : public Sink1<T> {
    public:
      SinkToConsole() {}

      virtual std::string_view self_typename() const override { return reflect::type_name<SinkToConsole<T>>(); }
      virtual std::string_view parent_typename() const override { return reflect::type_name<Sink1<T>>(); }
      virtual std::type_info const * parent_typeinfo() const override { return &typeid(Sink1<T>); }

      virtual void notify_ev(T const & ev) override {
	using logutil::operator<<;

	std::cout << ev << std::endl;
      } /*notify_ev*/
    }; /*SinkToConsole*/

    class TemporaryTest {
    public:
      static ref::rp<SinkToConsole<std::pair<xo::time::utc_nanos, double>>> realization_printer();
    }; /*TemporaryTest*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end Sink.hpp */
