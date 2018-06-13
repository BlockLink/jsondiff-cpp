#ifndef JSONDIFF_JSONDIFF_H
#define JSONDIFF_JSONDIFF_H

#include <jsondiff/config.h>
#include <jsondiff/diff_result.h>
#include <jsondiff/json_value_types.h>

#include <string>
#include <memory>

#include <fjson/io/json.hpp>
#include <fjson/string.hpp>
#include <fjson/variant.hpp>
#include <fjson/variant_object.hpp>


namespace jsondiff
{
	class JsonDiff
	{
	private:

	public:
		JsonDiff();
		virtual ~JsonDiff();


		DiffResultP diff_by_string(const std::string &old_json_str, const std::string &new_json_str);

		// ���������json�ַ�������Ҫֱ���������������������json_loadsΪjson��������diff����������ֱ�ӵ���diff_by_string����
		// @throws JsonDiffException
		DiffResultP diff(const JsonValue& old_json, const JsonValue& new_json);

		JsonValue patch_by_string(const std::string& old_json_value, DiffResultP diff_info);

		// �Ѿɰ汾��json,ʹ��diff�õ��°汾
		// @throws JsonDiffException
		JsonValue patch(const JsonValue& old_json, const DiffResultP& diff_info);

		JsonValue rollback_by_string(const std::string& new_json_value, DiffResultP diff_info);

		// ���°汾ʹ��diff�ع����ɰ汾
		// @throws JsonDiffException
		JsonValue rollback(const JsonValue& new_json, DiffResultP diff_info);

	};
}

#endif
