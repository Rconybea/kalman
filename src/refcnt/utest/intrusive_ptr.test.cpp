/* @file intrusive_ptr.test.cpp */

#include "refcnt/Refcounted.hpp"
#include "logutil/scope.hpp"
#include "catch2/catch.hpp"
#include <string_view>
#include <type_traits>

namespace xo {
  using xo::ref::Refcount;
  using xo::ref::Borrow;
  using xo::ref::rp;
  using xo::ref::brw;
  using xo::ref::intrusive_ptr_refcount;
  using xo::ref::intrusive_ptr_add_ref;
  using xo::ref::intrusive_ptr_release;

  namespace ut {
    namespace {
      static uint32_t ctor_count = 0;
      static uint32_t dtor_count = 0;

      /* empty object,  except for refcount */
      class JustRefcount : public ref::Refcount {
      public:
	JustRefcount() { ++ctor_count; }
	~JustRefcount() { ++dtor_count; }
      }; /*JustRefcount*/
    } /*namespace*/

    TEST_CASE("refcount", "[refcnt][trivial]") {
      REQUIRE(std::is_default_constructible<ref::Refcount>() == true);
      REQUIRE(std::has_virtual_destructor<ref::Refcount>() == true);

      /* refcount object self-initializes to 0 */
      Refcount x;
      REQUIRE(x.reference_counter() == 0);
    } /*TEST_CASE(refcount)*/

    TEST_CASE("null-intrusive-ptr", "[refcnt][trivial]") {
      //constexpr std::string_view c_self = "TEST_CASE:null-intrusive-ptr";

      REQUIRE(std::has_virtual_destructor<JustRefcount>() == true);

      rp<JustRefcount> p1;
      rp<JustRefcount> p2;

      REQUIRE(sizeof(p1) == sizeof(JustRefcount*));

      REQUIRE(p1.get() == nullptr);
      REQUIRE(p1.operator->() == nullptr);

      REQUIRE(p2.get() == nullptr);
      REQUIRE(p2.operator->() == nullptr);

      /* can assign a nullptr */
      rp<JustRefcount> p3;

      REQUIRE(p3.get() == nullptr);
      p3 = p1;
      REQUIRE(p3.get() == nullptr);

      /* can use aux functions on null pointers */
      REQUIRE(intrusive_ptr_refcount(p1.get()) == 0);

      intrusive_ptr_add_ref(nullptr);
      intrusive_ptr_release(nullptr);

      /* can borrow a null intrusive_ptr */
      brw<JustRefcount> p1_brw = p1.borrow();
      brw<JustRefcount> p2_brw = p2.borrow();

      REQUIRE(p1_brw.get() == nullptr);
      REQUIRE(p1_brw.operator->() == nullptr);
      /* null borrow is false-y */
      REQUIRE(p1_brw == false);

      /* can promote a borrowed pointer */
      rp<JustRefcount> pp = p1_brw.promote();

      REQUIRE(p1.get() == pp.get());

      /* comparisons */
      REQUIRE(Borrow<JustRefcount>::compare(p1_brw, p2_brw) == 0);
      REQUIRE(p1_brw == p2_brw);
      REQUIRE((p1_brw != p2_brw) == false);
      REQUIRE(p1 == p1_brw);
      REQUIRE((p1 != p1_brw) == false);
      REQUIRE(p1_brw == p1);
      REQUIRE((p1_brw != p1) == false);
    } /*TEST_CASE(null-intrusive_ptr)*/

    TEST_CASE("intrusive-ptr-identity", "[refcnt][identity]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1.get() == p1.operator->());
      REQUIRE(intrusive_ptr_refcount(p1.get()) == 1);
      REQUIRE(p1->reference_counter() == 1);

      intrusive_ptr_add_ref(p1.get());

      REQUIRE(intrusive_ptr_refcount(p1.get()) == 2);

      intrusive_ptr_release(p1.get());

      REQUIRE(intrusive_ptr_refcount(p1.get()) == 1);

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      rp<JustRefcount> p2(new JustRefcount());

      REQUIRE(ctor_count == cc + 2);
      REQUIRE(dtor_count == dc);

      REQUIRE(p2.get() != nullptr);
      REQUIRE(p2.get() != p1.get());
      REQUIRE(p2.get() == p2.operator->());
      REQUIRE(p2->reference_counter() == 1);

      /* can borrow a non-null intrusive-ptr */
      brw<JustRefcount> p1_brw = p1.borrow();

      REQUIRE(p1_brw.get() == p1.get());

      /* borrowing does not change refcount,  borrow not tracked */
      REQUIRE(ctor_count == cc + 2);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get()->reference_counter() == 1);

      /* copying borrowed pointer does not touch refcount */
      brw<JustRefcount> p1_brw2 = p1_brw;

      REQUIRE(ctor_count == cc + 2);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1_brw2.get() == p1.get());

      REQUIRE(p1.get()->reference_counter() == 1);
    } /*TEST_CASE(identity-intrusive-ptr)*/

    TEST_CASE("intrusive-ptr-release", "[refcnt][release]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1->reference_counter() == 1);

      /* reference count going to 0 -> delete object */
      p1 = nullptr;

      REQUIRE(p1.get() == nullptr);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc + 1);
    } /*TEST_CASE(intrusive-ptr-release)*/

    TEST_CASE("intrusive-ptr-copy", "[refcnt][copy]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());
      JustRefcount * p1_native = p1.get();
      
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1->reference_counter() == 1);
      
      /* copy ctor ran to make copy of p1,  did not allocate */
      rp<JustRefcount> p2(p1);

      REQUIRE(p1->reference_counter() == 2);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      
    } /*TEST_CASE(intrusive-ptr-copy)*/

    TEST_CASE("intrusive-ptr-move", "[refcnt][move]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());
      JustRefcount * p1_native = p1.get();

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1->reference_counter() == 1);
      
      rp<JustRefcount> p2{std::move(p1)};

      REQUIRE(p2->reference_counter() == 1);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p2 = nullptr;

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc + 1);
    } /*TEST_CASE(intrusive-ptr-move)*/

    TEST_CASE("instrusive-ptr-assign", "[refcnt][assign]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());
      JustRefcount * p1_native = p1.get();

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1->reference_counter() == 1);
      
      rp<JustRefcount> p2;

      REQUIRE(p2.get() == nullptr);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p2 = p1;

      REQUIRE(p2.get() == p1.get());
      REQUIRE(p2->reference_counter() == 2);

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p1 = nullptr;

      REQUIRE(p2->reference_counter() == 1);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p2 = nullptr;

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc + 1);
    } /*TEST_CASE(intrusive-ptr-assign)*/

    TEST_CASE("intrusive-ptr-move-assign", "[refcnt][move-assign]")
    {
      uint32_t cc = ctor_count;
      uint32_t dc = dtor_count;

      rp<JustRefcount> p1(new JustRefcount());
      JustRefcount * p1_native = p1.get();

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);
      REQUIRE(p1.get() != nullptr);
      REQUIRE(p1->reference_counter() == 1);
      
      rp<JustRefcount> p2;

      REQUIRE(p2.get() == nullptr);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p2 = std::move(p1);

      REQUIRE(p1.get() == nullptr);
      REQUIRE(p2.get() == p1_native);
      REQUIRE(p2->reference_counter() == 1);

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p1 = nullptr; /*no-op*/

      REQUIRE(p2->reference_counter() == 1);
      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc);

      p2 = nullptr;

      REQUIRE(ctor_count == cc + 1);
      REQUIRE(dtor_count == dc + 1);
    } /*TEST_CASE(intrusive-ptr-move-assign)*/
  } /*namespace ut*/
} /*namespace xo*/

/* end intrusive_ptr.test.cpp */
