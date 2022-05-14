/* @file tag.hpp */

#pragma once

#include <iostream>

// STRINGIFY(xyz) -> "xyz"
#define STRINGIFY(x) #x

// TAG(xyz) -> tag("xyz", xyz)
#define TAG(x) logutil::make_tag(STRINGIFY(x), x)
#define TAG2(x, y) logutil::make_tag(x, y)

namespace logutil {
  // associate a name with a value
  //
  // will print like
  //   :name value
  //*/
  template <typename Name, typename Value>
  struct tag {
    tag(Name const & n, Value const & v)
      : name_{n}, value_{v} {}

    Name const & name() const { return name_; }
    Value const & value() const { return value_; }

  private:
    Name name_;
    Value value_;
  }; /*tag*/

  /* deduce tag template-type from arguments */
  template<typename Name, typename Value>
  tag<Name, Value>
  make_tag(Name && n, Value && v)
  {
    return tag<Name, Value>(n, v);
  } /*make_tag*/

  template<typename Value>
  tag<char const *, Value>
  make_tag(char const * n, Value && v) {
    return tag<char const *, Value>(n, v);
  } /*make_tag*/

  template <typename Name, typename Value>
  inline std::ostream &
  operator<<(std::ostream &s,
	     tag<Name, Value> const & tag)
  {
    s << ":" << tag.name() << " " << tag.value();

    return s;
  } /*operator<<*/

} /*namespace logutil*/

/* end tag.hpp */
