#pragma once
#include <fjson/io/raw_variant.hpp>
#include <fjson/reflect/reflect.hpp>
#include <fjson/io/varint.hpp>
#include <fjson/optional.hpp>
#include <fjson/fwd.hpp>
#include <fjson/smart_ref_fwd.hpp>
#include <fjson/array.hpp>
#include <fjson/time.hpp>
#include <fjson/exception/exception.hpp>
#include <fjson/safe.hpp>
#include <fjson/io/raw_fwd.hpp>
#include <fjson/io/datastream.hpp>
#include <map>
#include <deque>
#include <iostream>

namespace fjson {
    namespace raw {
    template<typename Stream>
    inline void pack( Stream& s, const fjson::exception& e )
    {
       fjson::raw::pack( s, e.code() );
       fjson::raw::pack( s, std::string(e.name()) );
       fjson::raw::pack( s, std::string(e.what()) );
       fjson::raw::pack( s, e.get_log() );
    }
    template<typename Stream>
    inline void unpack( Stream& s, fjson::exception& e )
    {
       int64_t code;
       std::string name, what;
       log_messages msgs;

       fjson::raw::unpack( s, code );
       fjson::raw::unpack( s, name );
       fjson::raw::unpack( s, what );
       fjson::raw::unpack( s, msgs );

       e = fjson::exception( fjson::move(msgs), code, name, what );
    }

    template<typename Stream>
    inline void pack( Stream& s, const fjson::log_message& msg )
    {
       fjson::raw::pack( s, variant(msg) );
    }
    template<typename Stream>
    inline void unpack( Stream& s, fjson::log_message& msg )
    {
       fjson::variant vmsg;
       fjson::raw::unpack( s, vmsg );
       msg = vmsg.as<log_message>();
    }

    template<typename Stream>
    inline void pack( Stream& s, const fjson::path& tp )
    {
       fjson::raw::pack( s, tp.generic_string() );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fjson::path& tp )
    {
       std::string p;
       fjson::raw::unpack( s, p );
       tp = p;
    }

    template<typename Stream>
    inline void pack( Stream& s, const fjson::time_point_sec& tp )
    {
       uint32_t usec = tp.sec_since_epoch();
       s.write( (const char*)&usec, sizeof(usec) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fjson::time_point_sec& tp )
    { try {
       uint32_t sec;
       s.read( (char*)&sec, sizeof(sec) );
       tp = fjson::time_point() + fjson::seconds(sec);
    } FJSON_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream>
    inline void pack( Stream& s, const fjson::time_point& tp )
    {
       uint64_t usec = tp.time_since_epoch().count();
       s.write( (const char*)&usec, sizeof(usec) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fjson::time_point& tp )
    { try {
       uint64_t usec;
       s.read( (char*)&usec, sizeof(usec) );
       tp = fjson::time_point() + fjson::microseconds(usec);
    } FJSON_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream>
    inline void pack( Stream& s, const fjson::microseconds& usec )
    {
       uint64_t usec_as_int64 = usec.count();
       s.write( (const char*)&usec_as_int64, sizeof(usec_as_int64) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, fjson::microseconds& usec )
    { try {
       uint64_t usec_as_int64;
       s.read( (char*)&usec_as_int64, sizeof(usec_as_int64) );
       usec = fjson::microseconds(usec_as_int64);
    } FJSON_RETHROW_EXCEPTIONS( warn, "" ) }

    template<typename Stream, typename T, size_t N>
    inline void pack( Stream& s, const fjson::array<T,N>& v) {
      s.write((const char*)&v.data[0],N*sizeof(T));
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::shared_ptr<T>& v)
    {
      fjson::raw::pack( s, *v );
    }

    template<typename Stream, typename T, size_t N>
    inline void unpack( Stream& s, fjson::array<T,N>& v)
    { try {
      s.read((char*)&v.data[0],N*sizeof(T));
    } FJSON_RETHROW_EXCEPTIONS( warn, "fjson::array<type,length>", ("type",fjson::get_typename<T>::name())("length",N) ) }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::shared_ptr<T>& v)
    { try {
      v = std::make_shared<T>();
      fjson::raw::unpack( s, *v );
    } FJSON_RETHROW_EXCEPTIONS( warn, "std::shared_ptr<T>", ("type",fjson::get_typename<T>::name()) ) }

    template<typename Stream> inline void pack( Stream& s, const signed_int& v ) {
      uint32_t val = (v.value<<1) ^ (v.value>>31);
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      } while( val );
    }

    template<typename Stream> inline void pack( Stream& s, const unsigned_int& v ) {
      uint64_t val = v.value;
      do {
        uint8_t b = uint8_t(val) & 0x7f;
        val >>= 7;
        b |= ((val > 0) << 7);
        s.write((char*)&b,1);//.put(b);
      }while( val );
    }

    template<typename Stream> inline void unpack( Stream& s, signed_int& vi ) {
      uint32_t v = 0; char b = 0; int by = 0;
      do {
        s.get(b);
        v |= uint32_t(uint8_t(b) & 0x7f) << by;
        by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = ((v>>1) ^ (v>>31)) + (v&0x01);
      vi.value = v&0x01 ? vi.value : -vi.value;
      vi.value = -vi.value;
    }
    template<typename Stream> inline void unpack( Stream& s, unsigned_int& vi ) {
      uint64_t v = 0; char b = 0; uint8_t by = 0;
      do {
          s.get(b);
          v |= uint32_t(uint8_t(b) & 0x7f) << by;
          by += 7;
      } while( uint8_t(b) & 0x80 );
      vi.value = static_cast<uint32_t>(v);
    }

    template<typename Stream, typename T> inline void unpack( Stream& s, const T& vi )
    {
       T tmp;
       fjson::raw::unpack( s, tmp );
       FJSON_ASSERT( vi == tmp );
    }

    template<typename Stream> inline void pack( Stream& s, const char* v ) { fjson::raw::pack( s, fjson::string(v) ); }

    template<typename Stream, typename T>
    void pack( Stream& s, const safe<T>& v ) { fjson::raw::pack( s, v.value ); }

    template<typename Stream, typename T>
    void unpack( Stream& s, fjson::safe<T>& v ) { fjson::raw::unpack( s, v.value ); }

    template<typename Stream, typename T, unsigned int S, typename Align>
    void pack( Stream& s, const fjson::fwd<T,S,Align>& v ) {
       fjson::raw::pack( *v );
    }

    template<typename Stream, typename T, unsigned int S, typename Align>
    void unpack( Stream& s, fjson::fwd<T,S,Align>& v ) {
       fjson::raw::unpack( *v );
    }
    template<typename Stream, typename T>
    void pack( Stream& s, const fjson::smart_ref<T>& v ) { fjson::raw::pack( s, *v ); }

    template<typename Stream, typename T>
    void unpack( Stream& s, fjson::smart_ref<T>& v ) { fjson::raw::unpack( s, *v ); }

    // optional
    template<typename Stream, typename T>
    void pack( Stream& s, const fjson::optional<T>& v ) {
      fjson::raw::pack( s, bool(!!v) );
      if( !!v ) fjson::raw::pack( s, *v );
    }

    template<typename Stream, typename T>
    void unpack( Stream& s, fjson::optional<T>& v )
    { try {
      bool b; fjson::raw::unpack( s, b );
      if( b ) { v = T(); fjson::raw::unpack( s, *v ); }
    } FJSON_RETHROW_EXCEPTIONS( warn, "optional<${type}>", ("type",fjson::get_typename<T>::name() ) ) }

    // std::vector<char>
    template<typename Stream> inline void pack( Stream& s, const std::vector<char>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      if( value.size() )
        s.write( &value.front(), (uint32_t)value.size() );
    }
    template<typename Stream> inline void unpack( Stream& s, std::vector<char>& value ) {
      unsigned_int size; fjson::raw::unpack( s, size );
      FJSON_ASSERT( size.value < MAX_ARRAY_ALLOC_SIZE );
      value.resize(size.value);
      if( value.size() )
        s.read( value.data(), value.size() );
    }

    // fjson::string
    template<typename Stream> inline void pack( Stream& s, const fjson::string& v )  {
      fjson::raw::pack( s, unsigned_int((uint32_t)v.size()));
      if( v.size() ) s.write( v.c_str(), v.size() );
    }

    template<typename Stream> inline void unpack( Stream& s, fjson::string& v )  {
      std::vector<char> tmp; fjson::raw::unpack(s,tmp);
      if( tmp.size() )
         v = fjson::string(tmp.data(),tmp.data()+tmp.size());
      else v = fjson::string();
    }

    // bool
    template<typename Stream> inline void pack( Stream& s, const bool& v ) { fjson::raw::pack( s, uint8_t(v) );             }
    template<typename Stream> inline void unpack( Stream& s, bool& v )
    {
       uint8_t b;
       fjson::raw::unpack( s, b );
       FJSON_ASSERT( (b & ~1) == 0 );
       v=(b!=0);
    }

    namespace detail {

      template<typename Stream, typename Class>
      struct pack_object_visitor {
        pack_object_visitor(const Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        void operator()( const char* name )const {
          fjson::raw::pack( s, c.*p );
        }
        private:
          const Class& c;
          Stream&      s;
      };

      template<typename Stream, typename Class>
      struct unpack_object_visitor {
        unpack_object_visitor(Class& _c, Stream& _s)
        :c(_c),s(_s){}

        template<typename T, typename C, T(C::*p)>
        inline void operator()( const char* name )const
        { try {
          fjson::raw::unpack( s, c.*p );
        } FJSON_RETHROW_EXCEPTIONS( warn, "Error unpacking field ${field}", ("field",name) ) }
        private:
          Class&  c;
          Stream& s;
      };

      template<typename IsClass=fjson::true_type>
      struct if_class{
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) { s << v; }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) { s >> v; }
      };

      template<>
      struct if_class<fjson::false_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          s.write( (char*)&v, sizeof(v) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          s.read( (char*)&v, sizeof(v) );
        }
      };

      template<typename IsEnum=fjson::false_type>
      struct if_enum {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          fjson::reflector<T>::visit( pack_object_visitor<Stream,T>( v, s ) );
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          fjson::reflector<T>::visit( unpack_object_visitor<Stream,T>( v, s ) );
        }
      };
      template<>
      struct if_enum<fjson::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          fjson::raw::pack(s, (int64_t)v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          int64_t temp;
          fjson::raw::unpack(s, temp);
          v = (T)temp;
        }
      };

      template<typename IsReflected=fjson::false_type>
      struct if_reflected {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          if_class<typename fjson::is_class<T>::type>::pack(s,v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          if_class<typename fjson::is_class<T>::type>::unpack(s,v);
        }
      };
      template<>
      struct if_reflected<fjson::true_type> {
        template<typename Stream, typename T>
        static inline void pack( Stream& s, const T& v ) {
          if_enum< typename fjson::reflector<T>::is_enum >::pack(s,v);
        }
        template<typename Stream, typename T>
        static inline void unpack( Stream& s, T& v ) {
          if_enum< typename fjson::reflector<T>::is_enum >::unpack(s,v);
        }
      };

    } // namesapce detail

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::unordered_set<T>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::unordered_set<T>& value ) {
      unsigned_int size; fjson::raw::unpack( s, size );
      value.clear();
      FJSON_ASSERT( size.value*sizeof(T) < MAX_ARRAY_ALLOC_SIZE );
      value.reserve(size.value);
      for( uint32_t i = 0; i < size.value; ++i )
      {
          T tmp;
          fjson::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }


    template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::pair<K,V>& value ) {
       fjson::raw::pack( s, value.first );
       fjson::raw::pack( s, value.second );
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::pair<K,V>& value )
    {
       fjson::raw::unpack( s, value.first );
       fjson::raw::unpack( s, value.second );
    }

   template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::unordered_map<K,V>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::unordered_map<K,V>& value )
    {
      unsigned_int size; fjson::raw::unpack( s, size );
      value.clear();
      FJSON_ASSERT( size.value*(sizeof(K)+sizeof(V)) < MAX_ARRAY_ALLOC_SIZE );
      value.reserve(size.value);
      for( uint32_t i = 0; i < size.value; ++i )
      {
          std::pair<K,V> tmp;
          fjson::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }
    template<typename Stream, typename K, typename V>
    inline void pack( Stream& s, const std::map<K,V>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }
    template<typename Stream, typename K, typename V>
    inline void unpack( Stream& s, std::map<K,V>& value )
    {
      unsigned_int size; fjson::raw::unpack( s, size );
      value.clear();
      FJSON_ASSERT( size.value*(sizeof(K)+sizeof(V)) < MAX_ARRAY_ALLOC_SIZE );
      for( uint32_t i = 0; i < size.value; ++i )
      {
          std::pair<K,V> tmp;
          fjson::raw::unpack( s, tmp );
          value.insert( std::move(tmp) );
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::deque<T>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::deque<T>& value ) {
      unsigned_int size; fjson::raw::unpack( s, size );
      FJSON_ASSERT( size.value*sizeof(T) < MAX_ARRAY_ALLOC_SIZE );
      value.resize(size.value);
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::unpack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::vector<T>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::vector<T>& value ) {
      unsigned_int size; fjson::raw::unpack( s, size );
      FJSON_ASSERT( size.value*sizeof(T) < MAX_ARRAY_ALLOC_SIZE );
      value.resize(size.value);
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::unpack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void pack( Stream& s, const std::set<T>& value ) {
      fjson::raw::pack( s, unsigned_int((uint32_t)value.size()) );
      auto itr = value.begin();
      auto end = value.end();
      while( itr != end ) {
        fjson::raw::pack( s, *itr );
        ++itr;
      }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, std::set<T>& value ) {
      unsigned_int size; fjson::raw::unpack( s, size );
      for( uint64_t i = 0; i < size.value; ++i )
      {
        T tmp;
        fjson::raw::unpack( s, tmp );
        value.insert( std::move(tmp) );
      }
    }



    template<typename Stream, typename T>
    inline void pack( Stream& s, const T& v ) {
      fjson::raw::detail::if_reflected< typename fjson::reflector<T>::is_defined >::pack(s,v);
    }
    template<typename Stream, typename T>
    inline void unpack( Stream& s, T& v )
    { try {
      fjson::raw::detail::if_reflected< typename fjson::reflector<T>::is_defined >::unpack(s,v);
    } FJSON_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fjson::get_typename<T>::name() ) ) }

    template<typename T>
    inline size_t pack_size(  const T& v )
    {
      datastream<size_t> ps;
      fjson::raw::pack(ps,v );
      return ps.tellp();
    }

    template<typename T>
    inline std::vector<char> pack(  const T& v ) {
      datastream<size_t> ps;
      fjson::raw::pack(ps,v );
      std::vector<char> vec(ps.tellp());

      if( vec.size() ) {
        datastream<char*>  ds( vec.data(), size_t(vec.size()) );
        fjson::raw::pack(ds,v);
      }
      return vec;
    }

    template<typename T>
    inline T unpack( const std::vector<char>& s )
    { try  {
      T tmp;
      if( s.size() ) {
        datastream<const char*>  ds( s.data(), size_t(s.size()) );
        fjson::raw::unpack(ds,tmp);
      }
      return tmp;
    } FJSON_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fjson::get_typename<T>::name() ) ) }

    template<typename T>
    inline void unpack( const std::vector<char>& s, T& tmp )
    { try  {
      if( s.size() ) {
        datastream<const char*>  ds( s.data(), size_t(s.size()) );
        fjson::raw::unpack(ds,tmp);
      }
    } FJSON_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fjson::get_typename<T>::name() ) ) }

    template<typename T>
    inline void pack( char* d, uint32_t s, const T& v ) {
      datastream<char*> ds(d,s);
      fjson::raw::pack(ds,v );
    }

    template<typename T>
    inline T unpack( const char* d, uint32_t s )
    { try {
      T v;
      datastream<const char*>  ds( d, s );
      fjson::raw::unpack(ds,v);
      return v;
    } FJSON_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fjson::get_typename<T>::name() ) ) }
	
    template<typename T>
    inline void unpack( const char* d, uint32_t s, T& v )
    { try {
      datastream<const char*>  ds( d, s );
      fjson::raw::unpack(ds,v);
      return v;
    } FJSON_RETHROW_EXCEPTIONS( warn, "error unpacking ${type}", ("type",fjson::get_typename<T>::name() ) ) }

   template<typename Stream>
   struct pack_static_variant
   {
      Stream& stream;
      pack_static_variant( Stream& s ):stream(s){}

      typedef void result_type;
      template<typename T> void operator()( const T& v )const
      {
         fjson::raw::pack( stream, v );
      }
   };

   template<typename Stream>
   struct unpack_static_variant
   {
      Stream& stream;
      unpack_static_variant( Stream& s ):stream(s){}

      typedef void result_type;
      template<typename T> void operator()( T& v )const
      {
         fjson::raw::unpack( stream, v );
      }
   };


    template<typename Stream, typename... T>
    void pack( Stream& s, const static_variant<T...>& sv )
    {
       fjson::raw::pack( s, unsigned_int(sv.which()) );
       sv.visit( pack_static_variant<Stream>(s) );
    }

    template<typename Stream, typename... T> void unpack( Stream& s, static_variant<T...>& sv )
    {
       unsigned_int w;
       fjson::raw::unpack( s, w );
       sv.set_which(w.value);
       sv.visit( unpack_static_variant<Stream>(s) );
    }

} } // namespace fjson::raw

