/* @file Sink.test.cpp */

#include "reactor/Sink.hpp"
#include "catch2/catch.hpp"

namespace xo {
  using xo::reactor::AbstractSink;
  using xo::reactor::Sink1;
  using xo::reactor::SinkToConsole;
  using xo::time::utc_nanos;
  using xo::ref::rp;

  namespace {
    class TestSink : public Sink1<int> {
    public:
      TestSink() = default;

      virtual void notify_ev(int const & ev) override {}

      virtual std::string_view self_typename() const override { return reflect::type_name<TestSink>(); }
      virtual std::string_view parent_typename() const override { return reflect::type_name<Sink1<int>>(); }
      virtual std::type_info const * parent_typeinfo() const override { return &typeid(Sink1<int>); }
    }; /*TestSink*/

    class TestSink2 : public Sink1<utc_nanos> {
    public:
      TestSink2() = default;

      virtual void notify_ev(utc_nanos const & ev) override {}

      virtual std::string_view self_typename() const override { return reflect::type_name<TestSink2>(); }
      virtual std::string_view parent_typename() const override { return reflect::type_name<Sink1<utc_nanos>>(); }
      virtual std::type_info const * parent_typeinfo() const override { return &typeid(Sink1<utc_nanos>); }
    }; /*TestSink2*/

    using TestSink3 = SinkToConsole<std::pair<utc_nanos, double>>;
  } /*namespace*/

  namespace ut {
    TEST_CASE("sink-cast", "[sink]") {
      rp<TestSink> test_sink = new TestSink();
      rp<AbstractSink> sink = test_sink;

      TestSink * cast_sink = dynamic_cast<TestSink *>(sink.get());

      REQUIRE(test_sink.get() == cast_sink);

      Sink1<int> * int_sink = dynamic_cast<Sink1<int> *>(sink.get());

      REQUIRE(test_sink.get() == int_sink);

      Sink1<int> * int_sink2 = Sink1<int>::require_native("TEST_CASE(sink-cast)", sink.get());

      REQUIRE(test_sink.get() == int_sink2);
    } /*TEST_CASE(sink-cast)*/

    TEST_CASE("sink-cast2", "[sink]") {
      rp<TestSink2> test_sink = new TestSink2();
      rp<AbstractSink> sink = test_sink;

      TestSink2 * cast_sink = dynamic_cast<TestSink2 *>(sink.get());

      REQUIRE(test_sink.get() == cast_sink);

      Sink1<utc_nanos> * dt_sink = dynamic_cast<Sink1<utc_nanos> *>(sink.get());

      REQUIRE(test_sink.get() == dt_sink);

      Sink1<utc_nanos> * dt_sink2 = Sink1<utc_nanos>::require_native("TEST_CASE(sink-cast2)", sink.get());

      REQUIRE(test_sink.get() == dt_sink2);
    } /*TEST_CASE(sink-cast2)*/

    TEST_CASE("sink-cast3", "[sink]") {
      rp<TestSink3> test_sink = new TestSink3();
      rp<AbstractSink> sink = test_sink;

      TestSink3 * cast_sink = dynamic_cast<TestSink3 *>(sink.get());

      REQUIRE(test_sink.get() == cast_sink);

      Sink1<std::pair<utc_nanos, double>> * ev_sink
	= dynamic_cast<Sink1<std::pair<utc_nanos, double>> *>(sink.get());

      REQUIRE(test_sink.get() == ev_sink);

      Sink1<std::pair<utc_nanos, double>> * ev_sink2
	= Sink1<std::pair<utc_nanos, double>>::require_native("TEST_CASE(sink-cast3)", sink.get());

      REQUIRE(test_sink.get() == ev_sink2);
    } /*TEST_CASE(sink-cast3)*/
      
  } /*namespace ut*/
} /*namespace xo*/

/* end Sink.test.cpp */
