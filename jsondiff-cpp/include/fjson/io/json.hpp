#pragma once
#include <fjson/variant.hpp>
#include <iostream>
#include <sstream>
#include <fjson/io/datastream.hpp>

namespace fjson
{
	using std::ostream;
	using std::stringstream;

   /**
    *  Provides interface for json serialization.
    *
    *  json strings are always UTF8
    */
   class json
   {
      public:
         enum parse_type
         {
            legacy_parser         = 0,
            strict_parser         = 1,
            relaxed_parser        = 2,
            legacy_parser_with_string_doubles = 3
         };
         enum output_formatting
         {
            stringify_large_ints_and_doubles = 0,
            legacy_generator = 1
         };

         static ostream& to_stream( ostream& out, const fjson::string&);
         static ostream& to_stream( ostream& out, const variant& v, output_formatting format = stringify_large_ints_and_doubles );
         static ostream& to_stream( ostream& out, const variants& v, output_formatting format = stringify_large_ints_and_doubles );
         static ostream& to_stream( ostream& out, const variant_object& v, output_formatting format = stringify_large_ints_and_doubles );

         static variant  from_stream( std::istream& in, parse_type ptype = legacy_parser );

         static variant  from_string( const string& utf8_str, parse_type ptype = legacy_parser );
         static variants variants_from_string( const string& utf8_str, parse_type ptype = legacy_parser );
         static string   to_string( const variant& v, output_formatting format = stringify_large_ints_and_doubles );
         static string   to_pretty_string( const variant& v, output_formatting format = stringify_large_ints_and_doubles );

         static bool     is_valid( const std::string& json_str, parse_type ptype = legacy_parser );

         template<typename T>
         static string   to_string( const T& v, output_formatting format = stringify_large_ints_and_doubles ) 
         {
            return to_string( variant(v), format );
         }

         template<typename T>
         static string   to_pretty_string( const T& v, output_formatting format = stringify_large_ints_and_doubles ) 
         {
            return to_pretty_string( variant(v), format );
         }
   };

} // fjson
