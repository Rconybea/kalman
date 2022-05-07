/* @file scope.hpp */

#include <sstream>
#include <cstdint>

namespace logutil {

  class state_impl;

  // write x to stream s
  // note: here x is a universal reference, since
  //   (a) it's a template type
  //   (b) requires deduction to establish x's type
  // this means:
  //   x will be an r-value reference or an l-value reference
  // depending on calling context
  //
  // see:
  //   https://eli.thegreenplace.net/2014/variadic-templates-in-c/
  //   http://bajamircea.github.io/coding/cpp/2016/04/07/move-forward.html
  //   https://en.cppreference.com/w/cpp/language/value_category
  //
  //   has identity == has address
  //
  //     /- has identity -----------------\
  //     |                                |
  //     |  lvalue                        |
  //     |  glvalue         /------------------------------\
  //     |                  |             |                |
  //     |                  |  xvalue     |                |
  //     |                  |  rvalue     |                |
  //     |                  | glvalue     |                |
  //     |                  |             |                |
  //     \--------------------------------/                |
  //                        |                    rvalue    |
  //                        |                    prvalue   |
  //                        |                              |
  //                        \- can be moved ---------------/
  //
  // 1. has identity,  but cannot be moved -> it's an lvalue; otherwise it's an rvalue
  //    e.g: local variable name
  //
  // 2. can be moved,  but no identity -> it's a prvalue (pure right-value);
  //                                      otherwise it's a glvalue (generalized left-value)
  //    e.g: non-reference function return value,  or literal constant
  //
  // 3. has identity and can be moved -> it's an xvalue (strange value)
  //    e.g: std::move(a)
  //
  // reminder:
  // - std::move() does not move:  it converts lvalue to rvalue,  so compiler can select
  //   desired overload
  // - std::forward() does not forward: it recovers original value category
  //   (when starting with a universal reference),  so compiler can select
  //   desired ctor

  // Use:
  //   tos(s,a,b,c)
  // is the same as
  //   s << a << b << c;
  //
  template<class Stream, typename T>
  Stream & tos(Stream & s, T const && x) {
    s << x;
    return s;
  } /*tos*/

  template<class Stream, typename T, typename... Tn>
  Stream & tos(Stream & s, T const && x, Tn&&... rest) {
    s << x;
    return tos(s, rest...);
  } /*tos*/

  // nesting logger
  //
  // Use:
  //   void myfunc() {
  //     log::scope x;
  //     x.log(a,b,c);
  //     anotherfunc();
  //     x.log(d,e,f);
  //   }
  //
  //   void anotherfunc() {
  //     log::scope x;
  //     x.log(y);
  //   }
  //
  // output like:
  //    +myfunc:
  //     a,b,c
  //     +anotherfunc:
  //      y
  //     -anotherfunc:
  //     d,e,f
  //    -myfunc:
  //
  class scope {
  public:
    scope();
    ~scope();

    template<typename... Tn>
    void log(Tn&&... rest) { tos(require_thread_local_stream(), rest...); }

  private:
    /* establish stream for logging;  use thread-local storage for threadsafety */
    static std::stringstream & require_thread_local_stream();

    /* establish logging state;  use thread-local storage for threadsafety */
    static state_impl * require_thread_local_state();
  }; /*scope*/
} /*namespace logutil*/

/* end scope.hpp */
