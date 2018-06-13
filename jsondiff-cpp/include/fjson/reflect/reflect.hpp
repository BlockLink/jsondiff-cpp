#pragma once
/**
 * @file fjson/reflect.hpp
 *
 * @brief Defines types and macros used to provide reflection.
 *
 */

#include <fjson/utility.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <stdint.h>
#include <string.h>

#include <fjson/reflect/typename.hpp>

namespace fjson {

/**
 *  @brief defines visit functions for T
 *  Unless this is specialized, visit() will not be defined for T.
 *
 *  @tparam T - the type that will be visited.
 *
 *  The @ref FJSON_REFLECT(TYPE,MEMBERS) or FJSON_STATIC_REFLECT_DERIVED(TYPE,BASES,MEMBERS) macro is used to specialize this
 *  class for your type.
 */
template<typename T>
struct reflector{
    typedef T type;
    typedef fjson::false_type is_defined;
    typedef fjson::false_type is_enum;

    /**
     *  @tparam Visitor a function object of the form:
     *
     *    @code
     *     struct functor {
     *        template<typename Member, class Class, Member (Class::*member)>
     *        void operator()( const char* name )const;
     *     };
     *    @endcode
     *
     *  If T is an enum then the functor has the following form:
     *    @code
     *     struct functor {
     *        template<int Value>
     *        void operator()( const char* name )const;
     *     };
     *    @endcode
     *
     *  @param v a functor that will be called for each member on T
     *
     *  @note - this method is not defined for non-reflected types.
     */
    #ifdef DOXYGEN
    template<typename Visitor>
    static inline void visit( const Visitor& v );
    #endif // DOXYGEN
};

void throw_bad_enum_cast( int64_t i, const char* e );
void throw_bad_enum_cast( const char* k, const char* e );
} // namespace fjson


#ifndef DOXYGEN

#define FJSON_REFLECT_VISIT_BASE(r, visitor, base) \
  fjson::reflector<base>::visit( visitor );


#ifndef _MSC_VER
  #define TEMPLATE template
#else
  // Disable warning C4482: nonstandard extention used: enum 'enum_type::enum_value' used in qualified name
  #pragma warning( disable: 4482 )
  #define TEMPLATE
#endif

#define FJSON_REFLECT_VISIT_MEMBER( r, visitor, elem ) \
{ typedef decltype(((type*)nullptr)->elem) member_type;  \
  visitor.TEMPLATE operator()<member_type,type,&type::elem>( BOOST_PP_STRINGIZE(elem) ); \
}


#define FJSON_REFLECT_BASE_MEMBER_COUNT( r, OP, elem ) \
  OP fjson::reflector<elem>::total_member_count

#define FJSON_REFLECT_MEMBER_COUNT( r, OP, elem ) \
  OP 1

#define FJSON_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
static inline void visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
}

#define FJSON_REFLECT_DERIVED_IMPL_EXT( TYPE, INHERITS, MEMBERS ) \
template<typename Visitor>\
void fjson::reflector<TYPE>::visit( const Visitor& v ) { \
    BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_VISIT_BASE, v, INHERITS ) \
    BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_VISIT_MEMBER, v, MEMBERS ) \
}

#endif // DOXYGEN


#define FJSON_REFLECT_VISIT_ENUM( r, enum_type, elem ) \
  v.TEMPLATE operator()<enum_type::elem>(BOOST_PP_STRINGIZE(elem));
#define FJSON_REFLECT_ENUM_TO_STRING( r, enum_type, elem ) \
   case enum_type::elem: return BOOST_PP_STRINGIZE(elem);
#define FJSON_REFLECT_ENUM_TO_FJSON_STRING( r, enum_type, elem ) \
   case enum_type::elem: return fjson::string(BOOST_PP_STRINGIZE(elem));

#define FJSON_REFLECT_ENUM_FROM_STRING( r, enum_type, elem ) \
  if( strcmp( s, BOOST_PP_STRINGIZE(elem)  ) == 0 ) return enum_type::elem;


#define FJSON_REFLECT_ENUM( ENUM, FIELDS ) \
namespace fjson { \
template<> struct reflector<ENUM> { \
    typedef fjson::true_type is_defined; \
    typedef fjson::true_type is_enum; \
    static const char* to_string(ENUM elem) { \
      switch( elem ) { \
        BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_ENUM_TO_STRING, ENUM, FIELDS ) \
        default: \
           fjson::throw_bad_enum_cast( fjson::to_string(int64_t(elem)).c_str(), BOOST_PP_STRINGIZE(ENUM) ); \
      }\
      return nullptr; \
    } \
    static const char* to_string(int64_t i) { \
      return to_string(ENUM(i)); \
    } \
    static fjson::string to_fjson_string(ENUM elem) { \
      switch( elem ) { \
        BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_ENUM_TO_FJSON_STRING, ENUM, FIELDS ) \
      } \
      return fjson::to_string(int64_t(elem)); \
    } \
    static fjson::string to_fjson_string(int64_t i) { \
      return to_fjson_string(ENUM(i)); \
    } \
    static ENUM from_string( const char* s ) { \
        BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_ENUM_FROM_STRING, ENUM, FIELDS ) \
        return ENUM(atoi(s));\
    } \
};  \
}

/*  Note: FJSON_REFLECT_ENUM previously defined this function, but I don't think it ever
 *        did what we expected it to do.  I've disabled it for now.
 *
 *  template<typename Visitor> \
 *  static inline void visit( const Visitor& v ) { \
 *      BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_VISIT_ENUM, ENUM, FIELDS ) \
 *  }\
 */

/**
 *  @def FJSON_REFLECT_DERIVED(TYPE,INHERITS,MEMBERS)
 *
 *  @brief Specializes fjson::reflector for TYPE where
 *         type inherits other reflected classes
 *
 *  @param INHERITS - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define FJSON_REFLECT_DERIVED( TYPE, INHERITS, MEMBERS ) \
namespace fjson {  \
  template<> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef fjson::true_type  is_defined; \
    typedef fjson::false_type is_enum; \
    enum  member_count_enum {  \
      local_member_count = 0  BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_MEMBER_COUNT, +, MEMBERS ),\
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    FJSON_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; }
#define FJSON_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, INHERITS, MEMBERS ) \
namespace fjson {  \
  template<BOOST_PP_SEQ_ENUM(TEMPLATE_ARGS)> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
template<BOOST_PP_SEQ_ENUM(TEMPLATE_ARGS)> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef fjson::true_type  is_defined; \
    typedef fjson::false_type is_enum; \
    enum  member_count_enum {  \
      local_member_count = 0  BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_MEMBER_COUNT, +, MEMBERS ),\
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    FJSON_REFLECT_DERIVED_IMPL_INLINE( TYPE, INHERITS, MEMBERS ) \
}; }

//BOOST_PP_SEQ_SIZE(MEMBERS),

/**
 *  @def FJSON_REFLECT(TYPE,MEMBERS)
 *  @brief Specializes fjson::reflector for TYPE
 *
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 *
 *  @see FJSON_REFLECT_DERIVED
 */
#define FJSON_REFLECT( TYPE, MEMBERS ) \
    FJSON_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define FJSON_REFLECT_TEMPLATE( TEMPLATE_ARGS, TYPE, MEMBERS ) \
    FJSON_REFLECT_DERIVED_TEMPLATE( TEMPLATE_ARGS, TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define FJSON_REFLECT_EMPTY( TYPE ) \
    FJSON_REFLECT_DERIVED( TYPE, BOOST_PP_SEQ_NIL, BOOST_PP_SEQ_NIL )

#define FJSON_REFLECT_TYPENAME( TYPE ) \
namespace fjson { \
  template<> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
}

#define FJSON_REFLECT_FWD( TYPE ) \
namespace fjson { \
  template<> struct get_typename<TYPE>  { static const char* name()  { return BOOST_PP_STRINGIZE(TYPE);  } }; \
template<> struct reflector<TYPE> {\
    typedef TYPE type; \
    typedef fjson::true_type is_defined; \
    enum  member_count_enum {  \
      local_member_count = BOOST_PP_SEQ_SIZE(MEMBERS), \
      total_member_count = local_member_count BOOST_PP_SEQ_FOR_EACH( FJSON_REFLECT_BASE_MEMBER_COUNT, +, INHERITS )\
    }; \
    template<typename Visitor> static void visit( const Visitor& v ); \
}; }


#define FJSON_REFLECT_DERIVED_IMPL( TYPE, MEMBERS ) \
    FJSON_REFLECT_IMPL_DERIVED_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )

#define FJSON_REFLECT_IMPL( TYPE, MEMBERS ) \
    FJSON_REFLECT_DERIVED_IMPL_EXT( TYPE, BOOST_PP_SEQ_NIL, MEMBERS )



