/* @file Refcounted.hpp */

#pragma once

#include <boost/intrusive_ptr.hpp>
#include <atomic>

namespace xo {
  namespace refcnt {
    /* template alias for reference-counted (pointer, pointer-to-const) */
    template<typename T>
    using rp = boost::intrusive_ptr<T>;
    template<typename T>
    using rcp = boost::intrusive_ptr<T const>;

    template<typename T>
    class Refcounted;

    template<typename T>
    void intrusive_ptr_add_ref(Refcounted<T> *);

    template<typename T>
    void intrusive_ptr_release(Refcounted<T> *);

    template<typename T>
    class Refcounted {
    public:
      Refcounted() = default;

    private:
      friend void intrusive_ptr_add_ref<T>(Refcounted *);
      friend void intrusive_ptr_release<T>(Refcounted *);
      
    private:
      std::atomic<uint32_t> reference_counter_;
    }; /*Refcounted*/

    template<typename T>
    inline void
    intrusive_ptr_add_ref(Refcounted<T> * x) {
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

    template<typename T>
    inline void
    intrusive_ptr_release(Refcounted<T> * x) {
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
     */
    template<typename T>
    class Borrow {
    public:
      Borrow(rp<T> const & x) : ptr_(x.get()) {}
      Borrow(Borrow const & x) = default;

      T * get() const { return ptr_; }

      T & operator*() const { return *ptr_; }
      T * operator->() const { return ptr_; }

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
  } /*namespace refcnt*/
} /*namespace xo*/

/* end Refcounted.hpp */
