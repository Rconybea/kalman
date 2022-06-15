/* @file Refcounted.hpp */

#pragma once

#include "logutil/scope.hpp"
//#include <boost/intrusive_ptr.hpp>
#include <atomic>

namespace xo {
  namespace refcnt {
    class Refcount;

    /* originally used boost::instrusive_ptr<>.
     * ran into a bug.  probably mine,  but implemented
     * refcounting inline for debugging
     */
    template<typename T>
    class intrusive_ptr {
    public:
      intrusive_ptr() = default;
      intrusive_ptr(T * x) : ptr_(x) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const * c_self = "intrusive_ptr<>::ctor";
        constexpr bool c_logging_enabled = false;

        scope lscope(c_self, c_logging_enabled);
        if (c_logging_enabled)
          lscope.log("enter",
		     xtag("this", this),
		     xtag("x", x),
		     xtag("refcount(x)", static_cast<Refcount *>(x)),
		     xtag("n", intrusive_ptr_refcount(x)));

        intrusive_ptr_add_ref(ptr_);
      } /*ctor*/

      intrusive_ptr(intrusive_ptr<T> const & x) : ptr_(x.get()) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const * c_self = "intrusive_ptr<>::cctor";
        constexpr bool c_logging_enabled = false;

        scope lscope(c_self, c_logging_enabled);
        if (c_logging_enabled)
          lscope.log("enter",
		     xtag("this", this),
		     xtag("x", x.get()),
		     xtag("refcount(x)", static_cast<Refcount *>(x.get())),
		     xtag("n", intrusive_ptr_refcount(x.get())));

	intrusive_ptr_add_ref(ptr_);
      }

      ~intrusive_ptr() {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const * c_self = "intrusive_ptr<>::dtor";
        constexpr bool c_logging_enabled = false;

	T * x = this->ptr_;

	scope lscope(c_self, c_logging_enabled);
	if (c_logging_enabled)
	  lscope.log("enter",
		     xtag("this", this),
		     xtag("x", x),
		     xtag("refcount(x)", static_cast<Refcount *>(x)),
		     xtag("n", intrusive_ptr_refcount(this->ptr_)));

	this->ptr_ = nullptr;

	intrusive_ptr_release(x);
      } /*dtor*/
      
      static bool compare(intrusive_ptr<T> const & x,
			  intrusive_ptr<T> const & y)
      {
	return ptrdiff_t(x.get() - y.get());
      }

      T * get() const { return ptr_; }

      T * operator->() const { return ptr_; }

      intrusive_ptr<T> & operator=(intrusive_ptr<T> const & rhs) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const * c_self = "intrusive_ptr<>::assign";
        constexpr bool c_logging_enabled = false;

        scope lscope(c_self, c_logging_enabled);
        if (c_logging_enabled)
          lscope.log("enter",
		     xtag("rhs", rhs.get()));

	T * x = rhs.get();

	T * old = this->ptr_;
	this->ptr_ = x;

	intrusive_ptr_add_ref(x);
	intrusive_ptr_release(old);

	return *this;
      } /*operator=*/

    private:
      T * ptr_ = nullptr;
    }; /*intrusive_ptr*/

    template<typename T>
    inline bool operator==(intrusive_ptr<T> const & x, intrusive_ptr<T> const & y) { return intrusive_ptr<T>::compare(x, y) == 0; }

#ifdef SET_ASIDE
    /* template alias for reference-counted pointer */
    template<typename T>
    using rp = boost::intrusive_ptr<T>;
    /* template alias for reference-counted pointer-to-const */
    template<typename T>
    using rcp = boost::intrusive_ptr<T const>;
#else
    template<typename T>
    using rp = intrusive_ptr<T>;
#endif

    class Refcount {
    public:
      Refcount() : reference_counter_(0) {}
      /* WARNING: virtual dtor here is essential,
       *          since it's what allows us to invoke delete on a Refcount*,
       * for an object of some derived class type T.  Otherwise clang
       * will use different addresses for Refcount-part and T-part of
       * such instance,  which means pointer given to delete will not be
       * the same as pointer returned from new
       */
      virtual ~Refcount() = default;

      uint32_t reference_counter() { return reference_counter_.load(); }

    private:
      friend uint32_t intrusive_ptr_refcount(Refcount *);
      friend void intrusive_ptr_add_ref(Refcount *);
      friend void intrusive_ptr_release(Refcount *);
      
    private:
      std::atomic<uint32_t> reference_counter_;
    }; /*Refcount*/

    inline uint32_t
    intrusive_ptr_refcount(Refcount * x) {
      /* reporting accurately for diagnostics */
      return x->reference_counter_.load();
    } /*intrusive_ptr_refcount*/

    inline void
    intrusive_ptr_add_ref(Refcount * x) {
      /* for adding reference -- can use relaxed order,
       * since any reordering of a set of intrusive_ptr_add_ref()
       * calls is ok,   provided no intervening intrusive_ptr_release()
       * calls.
       */
      bool success = false;

      do {
	uint32_t n = x->reference_counter_.load(std::memory_order_relaxed);

	success = x->reference_counter_.compare_exchange_strong(n, n+1,
								std::memory_order_relaxed);
      } while(!success);
    } /*intrusive_ptr_add_ref*/

    /* WARNING -- broken if not templated here 
     */
    //template<typename T>
    inline void
    intrusive_ptr_release(Refcount * x) {
      using logutil::scope;
      using logutil::xtag;

      constexpr char const * c_self = "intrusive_ptr_release";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);
      if(c_logging_enabled)
	lscope.log("intrusive_ptr_release: enter",
		   xtag("x", x),
		   xtag("n", x->reference_counter_.load()));

      /* for decrement,  need acq_rel ordering */
      bool success = false;
      uint32_t n = 0;

      do {
	n = x->reference_counter_.load(std::memory_order_acquire);

	success = x->reference_counter_.compare_exchange_strong(n, n-1,
								std::memory_order_acq_rel);
      } while(!success);

      if(n == 1) {
	/* just deleted the last reference,  so recover the object */

	delete x;
      }
    } /*intrusive_ptr_release*/

    /* borrow a reference-counted pointer to pass down the stack
     * 1. borrowed pointer intended to replace:
     *     a. code like
     *          foo(rp<T> x),
     *        passing rp<T> by value requires increment/decrement pair,
     *        which is superfluous given that caller holds reference throughout
     *     b. code like
     *          foo(rp<T> const & x)
     *        passing rp<T> by reference requires double-indirection in called
     *        code
     * 2. borrowed pointer does not check/maintain reference count.
     *    it should never be stored in a struct;  intended strictly
     *    to be passed down stack
     * 3. just the same, want to be able to copy the borrowed pointer,
     *    to avoid double-indirection
     * 4. also can promote borrowed pointer to full reference-counted
     *    whenever desired
     */
    template<typename T>
    class Borrow {
    public:
      template<typename S>
      Borrow(rp<S> const & x) : ptr_(x.get()) {}
      Borrow(Borrow const & x) = default;

      T * get() const { return ptr_; }

      rp<T> promote() const { return rp<T>(ptr_); }

      T & operator*() const { return *ptr_; }
      T * operator->() const { return ptr_; }

      operator bool() const { return ptr_ != nullptr; }

      static int32_t compare(Borrow const & x, Borrow const & y) {
	return ptrdiff_t(x.get() - y.get());
      } /*compare*/

      static int32_t compare(rp<T> const & x, Borrow const & y) {
	return ptrdiff_t(x.get() - y.get());
      } /*compare*/

    private:
      T * ptr_ = nullptr;
    }; /*Borrow*/

    template<typename T>
    inline bool operator==(Borrow<T> x, Borrow<T> y) { return Borrow<T>::compare(x, y) == 0; }

    template<typename T>
    inline bool operator==(rp<T> const & x, Borrow<T> y) { return Borrow<T>::compare(x, y) == 0; }

    template<typename T>
    using brw = Borrow<T>;

#ifdef NOT_IN_USE
    template<typename T>
    class Refcounted : public Refcount {
    public:
      Refcounted() = default;
    }; /*Refcounted*/
#endif

  } /*namespace refcnt*/
} /*namespace xo*/

/* end Refcounted.hpp */
