#include <fjson/io/datastream.hpp>
#include <fjson/exception/exception.hpp>

NO_RETURN void fjson::detail::throw_datastream_range_error(char const* method, size_t len, int64_t over)
{
  FJSON_THROW_EXCEPTION( out_of_range_exception, "${method} datastream of length ${len} over by ${over}", ("method",fjson::string(method))("len",len)("over",over) );
}
