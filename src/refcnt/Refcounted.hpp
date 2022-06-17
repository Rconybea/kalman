/* @file Refcounted.hpp */

#pragma once

#include "logutil/scope.hpp"
#include "reflect/demangle.hpp"
//#include <boost/intrusive_ptr.hpp>
#include <atomic>
#include <cassert>

namespace xo {
  namespace ref {
    class Refcount;

    template<typename T>
    class Borrow;

    /* originally used boost::instrusive_ptr<>.
     * ran into a bug.  probably mine,  but implemented
     * refcounting inline for debugging
     */
    template<typename T>
    class intrusive_ptr {
    public:
      intrusive_ptr() : ptr_(nullptr) {} 
      intrusive_ptr(T * x) : ptr_(x) {
	intrusive_ptr_log_ctor(sc_self_type, this, x);
        intrusive_ptr_add_ref(ptr_);
      } /*ctor*/

      /* NOTE: need exactly this form for copy-constructor
       *       clang11 will not recognize template form below as
       *       supplying copy ctor,   and default version is broken for
       *       instrusive_ptr.
       */
      intrusive_ptr(intrusive_ptr const & x) : ptr_(x.get()) {
	intrusive_ptr_log_cctor(sc_self_type, this, x.get());
	intrusive_ptr_add_ref(ptr_);
      } /*cctor*/

      /* create from instrusive pointer to some related type S */
      template<typename S>
      intrusive_ptr(intrusive_ptr<S> const & x) : ptr_(x.get()) {
	intrusive_ptr_log_cctor(sc_self_type, this, x.get());
	intrusive_ptr_add_ref(ptr_);
      } /*cctor*/

      /* move ctor -- in this case don't need to update refcount */
      intrusive_ptr(intrusive_ptr && x) : ptr_{std::move(x.ptr_)} {
	/* since we're moving from x,  need to make sure x dtor
	 * doesn't decrement refcount
	 */
	x.ptr_ = nullptr;
      }

      ~intrusive_ptr() {
	T * x = this->ptr_;

	intrusive_ptr_log_dtor(sc_self_type, this, x);

	this->ptr_ = nullptr;

	intrusive_ptr_release(x);
      } /*dtor*/
      
      static bool compare(intrusive_ptr<T> const & x,
			  intrusive_ptr<T> const & y)
      {
	return ptrdiff_t(x.get() - y.get());
      }

      Borrow<T> borrow() const;

      T * get() const { return ptr_; }

      T * operator->() const { return ptr_; }

      intrusive_ptr<T> & operator=(intrusive_ptr<T> const & rhs) {
	T * x = rhs.get();

	intrusive_ptr_log_assign(sc_self_type, this, x);

	T * old = this->ptr_;
	this->ptr_ = x;

	intrusive_ptr_add_ref(x);
	intrusive_ptr_release(old);

	return *this;
      } /*operator=*/

      intrusive_ptr<T> & operator=(intrusive_ptr<T> && rhs) {
	std::swap(this->ptr_, rhs.ptr_);

	/* dtor on rhs will decrement refcount on old value of this->ptr_ */

	return *this;
      } /*operator=*/

    private:
      static constexpr std::string_view sc_self_type = xo::reflect::type_name<intrusive_ptr<T>>();

    private:
      T * ptr_ = nullptr;
    }; /*intrusive_ptr*/

    template<typename T>
    inline bool operator==(intrusive_ptr<T> const & x, intrusive_ptr<T> const & y) { return intrusive_ptr<T>::compare(x, y) == 0; }

    template<typename T>
    using rp = intrusive_ptr<T>;

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
      if (x)
	return x->reference_counter_.load();
      else
	return 0;
    } /*intrusive_ptr_refcount*/

    void intrusive_ptr_log_ctor(std::string_view const & self_type,
				void * this_ptr,
				Refcount * x);
    void intrusive_ptr_log_cctor(std::string_view const & self_type,
				 void * this_ptr,
				 Refcount * x);
    void intrusive_ptr_log_dtor(std::string_view const &self_type,
				void * this_ptr,
                                Refcount * x);
    void intrusive_ptr_log_assign(std::string_view const &self_type,
				  void * this_ptr,
				  Refcount * x);
    void intrusive_ptr_add_ref(Refcount * x);
    void intrusive_ptr_release(Refcount * x);

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

    template<typename T>
    Borrow<T>
    intrusive_ptr<T>::borrow() const {
      return Borrow<T>(*this);
    } /*borrow*/

  } /*namespace ref*/
} /*namespace xo*/

/* end Refcounted.hpp */
