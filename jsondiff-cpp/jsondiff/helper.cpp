#include <jsondiff/helper.h>

namespace jsondiff
{
	namespace utils
	{
		bool string_ends_with(std::string str, std::string end)
		{
			auto pos = str.find_first_of(end);
			return pos >= 0 && (pos + end.size() == str.size());
		}

		std::string string_without_ext(std::string str, std::string ext)
		{
			return str.substr(0, str.find_last_of(ext) - ext.size() + 1);
		}
	}
}
