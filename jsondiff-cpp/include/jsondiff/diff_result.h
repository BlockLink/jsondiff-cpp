#ifndef JSONDIFF_DIFF_RESULT_H
#define JSONDIFF_DIFF_RESULT_H

#include <string>
#include <memory>
#include <jsondiff/json_value_types.h>

namespace jsondiff
{

	

	class DiffResult
	{
	private:
		JsonValue _diff_json;
		bool _is_undefined;
	public:
		DiffResult();
		DiffResult(JsonValue diff_json);
		virtual ~DiffResult();

		std::string str() const;
		std::string pretty_str() const;
		bool is_undefined() const;

		JsonValue value() const;

		// �� json diffת���Ѻÿɶ����ַ���
		std::string pretty_diff_str(size_t indent_count=0) const;

	public:
		static std::shared_ptr<DiffResult> make_undefined_diff_result();
	};

	typedef std::shared_ptr<DiffResult> DiffResultP;
}

#endif
