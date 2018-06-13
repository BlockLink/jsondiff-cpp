#pragma once
#include <fjson/reflect/reflect.hpp>
#include <fjson/io/raw_fwd.hpp>
#include <fjson/variant.hpp>

namespace fjson
{
  template<typename IntType, typename EnumType>
  class enum_type
  {
    public:
      enum_type( EnumType t )
      :value(t){}
      
      enum_type( IntType t )
      :value( (EnumType)t ){}
      
      enum_type(){}
      
      explicit operator IntType()const     { return static_cast<IntType>(value);    }
      operator EnumType()const    { return value;                          }
      operator std::string()const { return fjson::reflector<EnumType>::to_string(value); }
      
      enum_type& operator=( IntType i )  { value = (EnumType)i; return *this;}
      enum_type& operator=( EnumType i ) { value = i; return *this;}
      bool       operator<( EnumType i ) const { return value < i; }
      bool       operator>( EnumType i ) const { return value < i; }

      bool operator<(const enum_type& e) const { return value < e.value;}
      bool operator>(const enum_type& e) const { return value > e.value;}

      bool operator<=(const enum_type& e) const { return value <= e.value;}
      bool operator>=(const enum_type& e) const { return value >= e.value;}

      friend bool operator==( const enum_type& e, IntType i ){ return e.value == (EnumType)i;}
      friend bool operator==( const enum_type& e, EnumType i ){ return e.value == i;         }

      friend bool operator==( const enum_type& e, const enum_type& i ){ return e.value == i.value;  }
      friend bool operator==( IntType i, const enum_type& e){ return e.value == (EnumType)i; }
      friend bool operator==( EnumType i, const enum_type& e ){ return e.value == i;         }

      friend bool operator!=( const enum_type& e, IntType i ){ return e.value != (EnumType)i;}
      friend bool operator!=( const enum_type& e, EnumType i ){ return e.value != i;         }
      friend bool operator!=( const enum_type& e, const enum_type& i ){ return e.value != i.value;  }

      EnumType value;
  };


  template<typename IntType, typename EnumType>
  void to_variant( const enum_type<IntType,EnumType>& var,  variant& vo )
  {
    vo = (EnumType)var.value;
  }
  template<typename IntType, typename EnumType>
  void from_variant( const variant& var,  enum_type<IntType,EnumType>& vo )
  {
    vo.value = var.as<EnumType>();
  }


  /** serializes like an IntType */
  namespace raw 
  { 
    template<typename Stream, typename IntType, typename EnumType>
    inline void pack( Stream& s, const fjson::enum_type<IntType,EnumType>& tp )
    {
       fjson::raw::pack( s, static_cast<IntType>(tp) );
    }

    template<typename Stream, typename IntType, typename EnumType>
    inline void unpack( Stream& s, fjson::enum_type<IntType,EnumType>& tp )
    {
       IntType t;
       fjson::raw::unpack( s, t );
       tp = t;
    }
  }

}


