#ifndef JSONDIFF_HELPER_H
#define JSONDIFF_HELPER_H

#include <jsondiff/config.h>

#include <fc/io/json.hpp>
#include <fc/string.hpp>
#include <fc/variant.hpp>
#include <fc/variant_object.hpp>

namespace jsondiff
{
	namespace utils
	{
		bool string_ends_with(std::string str, std::string end);

		// �ҵ�һ���ַ���strȡ����׺ext��ʣ����ַ���
		std::string string_without_ext(std::string str, std::string ext);
	}
}

#endif
