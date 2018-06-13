#pragma once

#include <fjson/utility.hpp>
#include <fjson/smart_ref_fwd.hpp>
#include <new>

namespace fjson {

    namespace detail {

      template<typename A, typename U>
      struct insert_op {
        typedef decltype( *((A*)0) << *((typename fjson::remove_reference<U>::type*)0) ) type; 
      };

      template<typename A, typename U>
      struct extract_op {
        A* a;
        U* u;
        typedef decltype( *a >> *u ) type;
      };
    }

    template<typename T, typename U>
    auto operator << ( U& u, const smart_ref<T>& f ) -> typename detail::insert_op<U,T>::type { return u << *f; }

    template<typename T, typename U>
    auto operator >> ( U& u, smart_ref<T>& f ) -> typename detail::extract_op<U,T>::type { return u >> *f; }

    template<typename T>
    bool smart_ref<T>::operator !()const { return !(**this); }

    template<typename T>
    template<typename U>
    smart_ref<T>::smart_ref( U&& u ) {
      impl = new (this) T( fjson::forward<U>(u) );
    }

    template<typename T>
    template<typename U,typename V>
    smart_ref<T>::smart_ref( U&& u, V&& v ) {
      impl = new T( fjson::forward<U>(u), fjson::forward<V>(v) );
    }
    template<typename T>
    template<typename U,typename V,typename X,typename Y>
    smart_ref<T>::smart_ref( U&& u, V&& v, X&& x, Y&&  y ) {
      impl = new T( fjson::forward<U>(u), fjson::forward<V>(v), fjson::forward<X>(x), fjson::forward<Y>(y) );
    }

    template<typename T>
    smart_ref<T>::smart_ref() {
      impl = new T;
    }
    template<typename T>
    smart_ref<T>::smart_ref( const smart_ref<T>& f ){
      impl = new T( *f );
    }
    template<typename T>
    smart_ref<T>::smart_ref( smart_ref<T>&& f ){
      impl = new T( fjson::move(*f) );
    }

    template<typename T>
    smart_ref<T>::operator  T&() { return *impl; }
    template<typename T>
    smart_ref<T>::operator const T&()const { return *impl; }

    template<typename T>
    T& smart_ref<T>::operator*() { return *impl; }
    template<typename T>
    const T& smart_ref<T>::operator*()const  { return *impl; }
    template<typename T>
    const T* smart_ref<T>::operator->()const { return impl; }

    template<typename T>
    T* smart_ref<T>::operator->(){ return impl; }

    template<typename T>
    smart_ref<T>::~smart_ref() {
       delete impl;
    }

    template<typename T>
    template<typename U>
    T& smart_ref<T>::operator = ( U&& u ) {
      return **this = fjson::forward<U>(u);
    }

    template<typename T>
    T& smart_ref<T>::operator = ( smart_ref<T>&& u ) {
      if( &u == this ) return *impl;
      if( impl ) delete impl;
      impl = u.impl;
      u.impl = nullptr;
      return *impl;
    }

    template<typename T>
    T& smart_ref<T>::operator = ( const smart_ref<T>& u ) {
      if( &u == this ) return *impl;
      return **this = *u;
    }

} // namespace fjson

