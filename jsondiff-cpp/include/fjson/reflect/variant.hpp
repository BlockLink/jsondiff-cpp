#pragma once
#include <fjson/reflect/reflect.hpp>
#include <fjson/variant_object.hpp>

namespace fjson
{
   template<typename T>
   void to_variant( const T& o, variant& v );
   template<typename T>
   void from_variant( const variant& v, T& o );


   template<typename T>
   class to_variant_visitor
   {
      public:
         to_variant_visitor( mutable_variant_object& mvo, const T& v )
         :vo(mvo),val(v){}

         template<typename Member, class Class, Member (Class::*member)>
         void operator()( const char* name )const
         {
            this->add(vo,name,(val.*member));
         }

      private:
         template<typename M>
         void add( mutable_variant_object& vo, const char* name, const optional<M>& v )const
         { 
            if( v.valid() )
               vo(name,*v);
         }
         template<typename M>
         void add( mutable_variant_object& vo, const char* name, const M& v )const
         { vo(name,v); }

         mutable_variant_object& vo;
         const T& val;
   };

   template<typename T>
   class from_variant_visitor
   {
      public:
         from_variant_visitor( const variant_object& _vo, T& v )
         :vo(_vo),val(v){}

         template<typename Member, class Class, Member (Class::*member)>
         void operator()( const char* name )const
         {
            auto itr = vo.find(name);
            if( itr != vo.end() )
               from_variant( itr->value(), val.*member );
         }

         const variant_object& vo;
         T& val;
   };

   template<typename IsReflected=fjson::false_type>
   struct if_enum 
   {
     template<typename T>
     static inline void to_variant( const T& v, fjson::variant& vo ) 
     { 
         mutable_variant_object mvo;
         fjson::reflector<T>::visit( to_variant_visitor<T>( mvo, v ) );
         vo = fjson::move(mvo);
     }
     template<typename T>
     static inline void from_variant( const fjson::variant& v, T& o ) 
     { 
         const variant_object& vo = v.get_object();
         fjson::reflector<T>::visit( from_variant_visitor<T>( vo, o ) );
     }
   };

    template<> 
    struct if_enum<fjson::true_type> 
    {
       template<typename T>
       static inline void to_variant( const T& o, fjson::variant& v ) 
       { 
           v = fjson::reflector<T>::to_fjson_string(o);
       }
       template<typename T>
       static inline void from_variant( const fjson::variant& v, T& o ) 
       { 
           if( v.is_string() )
              o = fjson::reflector<T>::from_string( v.get_string().c_str() );
           else 
              o = static_cast<T>(v.as_int64());
       }
    };


   template<typename T>
   void to_variant( const T& o, variant& v )
   {
      if_enum<typename fjson::reflector<T>::is_enum>::to_variant( o, v );
   }

   template<typename T>
   void from_variant( const variant& v, T& o )
   {
      if_enum<typename fjson::reflector<T>::is_enum>::from_variant( v, o );
   }

}
