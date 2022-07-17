/* @file TypeDescr.hpp */

#include "reflect/demangle.hpp"
#include <string_view>
#include <cstdint>

namespace xo {
  namespace reflect {
    /* A reflected type is a type for which we keep information around at runtime
     * Assign reflected types unique (within an executable) ids,
     * allocating consecutively, starting from 1.
     * Reserve 0 as a sentinel
     */
    class TypeId {
    public:
      /* allocate a new TypeId value.
       * promise:
       * - retval.id() > 0
       */
      static TypeId allocate() { return TypeId(s_next_id++); }

      std::uint32_t id() const { return id_; }

    private:
      explicit TypeId(std::uint32_t id) : id_{id} {}

    private:
      static std::uint32_t s_next_id;
      
      /* unique index# for this type.
       * 0 reserved for sentinel
       */
      std::uint32_t id_ = 0;
    }; /*TypeId*/

    class TypeDescr {
    public:
      /* type-description objects for a type T is unique,
       *  --> can always use its address
       */
      TypeDescr(TypeDescr const & x) = delete;
      
    protected:
      TypeDescr(TypeId id, std::string_view canonical_name)
	: id_{std::move(id)},
	  canonical_name_{std::move(canonical_name)} {}

      TypeId id() const { return id_; }
      std::string_view const & canonical_name() const { return canonical_name_; }

    private:
      /* vector of all TypeDescr instances.  singleton */
      static std::vector<TypeDescr> s_type_table_v;

    private:
      /* unique id# for this type */
      TypeId id_;
      /* canonical name for this type (see demangle.hpp for type_name<T>()) */
      std::string_view canonical_name_;
    }; /*TypeDescr*/

    template<typename T>
    class EstablishTypeDescr : public TypeDescr {
    public:
      TypeDescr * establish() { return &s_type_descr; }
	
    private:
      EstablishTypeDescr() : TypeDescr(TypeId::allocate(),
				       type_name<T>() /*canonical_name*/) {}

    private:
      /* unique type-description object for type T */
      static EstablishTypeDescr<T> s_type_descr;
    }; /*EstablishTypeDescr*/
  } /*namespace reflect*/
} /*namespace xo*/

/* end TypeDescr.hpp */
