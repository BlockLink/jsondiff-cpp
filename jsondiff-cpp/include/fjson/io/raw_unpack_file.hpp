#pragma once
#include <fjson/io/raw.hpp>
#include <fjson/interprocess/file_mapping.hpp>
#include <fjson/filesystem.hpp>
#include <fjson/exception/exception.hpp>

namespace fjson
{
    namespace raw
    {
        template<typename T>
        void unpack_file( const fjson::path& filename, T& obj )
        {
           try {
               fjson::file_mapping fmap( filename.generic_string().c_str(), fjson::read_only);
               fjson::mapped_region mapr( fmap, fjson::read_only, 0, fjson::file_size(filename) );
               auto cs  = (const char*)mapr.get_address();

               fjson::datastream<const char*> ds( cs, mapr.get_size() );
               fjson::raw::unpack(ds,obj);
           } FJSON_RETHROW_EXCEPTIONS( info, "unpacking file ${file}", ("file",filename) );
        }
   }
}
