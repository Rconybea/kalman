/* @file scope.hpp */

#pragma once

#include "tag.hpp"

#include <iostream>
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
  Stream & tos(Stream & s, T && x) {
    s << x;
    return s;
  } /*tos*/

  template<class Stream, typename T, typename... Tn>
  Stream & tos(Stream & s, T && x, Tn&&... rest) {
    s << x;
    return tos(s, rest...);
  } /*tos*/

  // like tos(..),  but append newline
  //
  template<class Stream, typename... Tn>
  Stream & tosn(Stream & s, Tn&&... args) {
    tos(s, args...);
    s << std::endl;
    return s;
  } /*tosn*/

  // nesting logger
  //
  // Use:
  //   using logutil::scope;
  //
  //   void myfunc() {
  //     scope x;
  //     x.log(a,b,c);
  //     anotherfunc();
  //     x.log(d,e,f);
  //   }
  //
  //   void anotherfunc() {
  //     scope x;
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
    scope(char const * fn);
    ~scope();

    template<typename... Tn>
    void log(Tn&&... rest) {
      if(this->finalized_) {
	throw std::runtime_error("scope: attempt to use finalized scope");
      } else {
	state_impl * logstate = require_indent_thread_local_state();

	/* log to per-thread stream to prevent data races */
	tosn(logstate2stream(logstate), rest...);
	//tosn(std::cout, "debug>", rest...);

	this->flush2clog(logstate);
      }
    } /*log*/

    /* call once to end scope before dtor */
    void end_scope();

  private:
    /* establish stream for logging;  use thread-local storage for threadsafety.
     * stream, if recycled (i.e. after 1st call to scope.log() from a particular thread),
     * will be in 'reset-to-beginning of buffer' state.
     */
    static state_impl * require_indent_thread_local_state();

    /* establish logging state;  use thread-local storage for threadsafety */
    static state_impl * require_thread_local_state();

    /* retrieve permanently-associated ostream for logging-state */
    static std::ostream & logstate2stream(state_impl * logstate);

    /* write collected output to std::clog */
    void flush2clog(state_impl * logstate);

  private:
    /* logging state (strictly: not needed,  since will be thread-local) */
    state_impl * logstate_ = nullptr;
    /* name of this scope */
    char const * name_ = "<anonymous>";
    /* set once per scope */
    bool finalized_ = false;
  }; /*scope*/
} /*namespace logutil*/

/* end scope.hpp */
