#include <jsondiff/jsondiff.h>
#include <jsondiff/exceptions.h>
#include <jsondiff/diff_result.h>
#include <jsondiff/helper.h>

#include <fc/io/json.hpp>
#include <fc/string.hpp>
#include <fc/variant.hpp>
#include <fc/variant_object.hpp>

namespace jsondiff
{
	JsonDiff::JsonDiff()
	{

	}

	JsonDiff::~JsonDiff()
	{

	}

	DiffResultP JsonDiff::diff_by_string(const std::string &old_json_str, const std::string &new_json_str)
	{
		return diff(json_loads(old_json_str), json_loads(new_json_str));
	}

	DiffResultP JsonDiff::diff(const JsonValue& old_json, const JsonValue& new_json)
	{
		auto old_json_type = guess_json_value_type(old_json);
		auto new_json_type = guess_json_value_type(new_json);

		if (is_scalar_json_value_type(old_json_type) || old_json_type != new_json_type)
		{
			// oldֵ�ǻ������� ��old��newֵ�����Ͳ�һ��
			// should return undefined for two identical values
			// should return { __old: <old value>, __new : <new value> } object for two different numbers
			if (is_scalar_json_value_type(old_json_type) && old_json_type == new_json_type) {
				if (JVT_STRING == old_json_type) {
					const auto& old_str = old_json.as_string();
					const auto& new_str = new_json.as_string();
					if (old_str.size() != new_str.size() || old_str != new_str) {
						fc::mutable_variant_object result_json;
						result_json[JSONDIFF_KEY_OLD_VALUE] = old_json;
						result_json[JSONDIFF_KEY_NEW_VALUE] = new_json;
						return std::make_shared<DiffResult>(result_json);
					}
					else {
						// identical scalar values
						return DiffResult::make_undefined_diff_result();
					}
				}
				else if (json_dumps(old_json) != json_dumps(new_json)) {
					fc::mutable_variant_object result_json;
					result_json[JSONDIFF_KEY_OLD_VALUE] = old_json;
					result_json[JSONDIFF_KEY_NEW_VALUE] = new_json;
					return std::make_shared<DiffResult>(result_json);
				}
				else {
					// identical scalar values
					return DiffResult::make_undefined_diff_result();
				}
			}
			else if (old_json_type != new_json_type)
			{
				fc::mutable_variant_object result_json;
				result_json[JSONDIFF_KEY_OLD_VALUE] = old_json;
				result_json[JSONDIFF_KEY_NEW_VALUE] = new_json;
				return std::make_shared<DiffResult>(result_json);
			}
			else
			{
				// identical scalar values
				return DiffResult::make_undefined_diff_result();
			}
		}
		else if (old_json_type == JsonValueType::JVT_OBJECT)
		{
			// old��new����object����ʱ
			// should return undefined for two objects with identical contents
			// should return undefined for two object hierarchies with identical contents
			// should return { <key>__deleted: <old value> } when the second object is missing a key
			// should return { <key>__added: <new value> } when the first object is missing a key
			// should return { <key>: { __old: <old value>, __new : <new value> } } for two objects with diffent scalar values for a key
			// should return { <key>: <diff> } with a recursive diff for two objects with diffent values for a key
			auto a_obj = old_json.as<fc::mutable_variant_object>();
			auto b_obj = new_json.as<fc::mutable_variant_object>();
			bool same = false;
			fc::mutable_variant_object diff_json;
			for (auto i = a_obj.begin(); i != a_obj.end(); i++)
			{
				auto a_i_value = i->value();
				auto a_i_key = i->key();
				if (b_obj.find(a_i_key) == b_obj.end())
				{
					// ������old��������new
					diff_json[a_i_key + JSONDIFF_KEY_DELETED_POSTFIX] = a_i_value;
				}
				else
				{
					// old��new�ж������key
					auto sub_diff_value = diff(a_i_value, b_obj[a_i_key]);
					if (sub_diff_value->is_undefined()) // һ����Ԫ��
						continue;
					// �޸�
					diff_json[a_i_key] = sub_diff_value->value();
				}
			}
			for (auto j = b_obj.begin(); j != b_obj.end(); j++)
			{
				auto key = j->key();
				if (a_obj.find(key) == a_obj.end())
				{
					// ��������old���Ǵ�����new
					diff_json[key + JSONDIFF_KEY_ADDED_POSTFIX] = j->value();
				}
			}
			if (diff_json.size() < 1)
				return DiffResult::make_undefined_diff_result();
			return std::make_shared<DiffResult>(diff_json);
		}
		else if (old_json_type == JsonValueType::JVT_ARRAY)
		{
			// old��new����array����ʱ
			// with arrays of scalars
			//   should return undefined for two arrays with identical contents
			//   should return[..., ['-', remove_from_position_index, <removed item>], ...] for two arrays when the second array is missing a value
			//   should return[..., ['+', insert_position_index, <added item>], ...] for two arrays when the second one has an extra value
			//   should return[..., ['+', insert_position_index, <added item>]] for two arrays when the second one has an extra value at the end(edge case test)
			// with arrays of objects
			//   should return undefined for two arrays with identical contents
			//   should return[..., ['-', remove_from_position_index, <removed item>], ...] for two arrays when the second array is missing a value
			//   should return[..., ['+', insert_position_index, <added item>], ...] for two arrays when the second array has an extra value
			//   should return[..., ['~', position_index, <diff>], ...] for two arrays when an item has been modified(note: involves a crazy heuristic)

			auto a_array = old_json.as<fc::variants>();
			auto b_array = new_json.as<fc::variants>();

			// TODO: ������array�Ĵ󲿷�Ԫ����ͬʱ�����ǿ���ǰ�����벿��Ԫ�أ���ʱ��Ӧ�þ�������diff��С

			// һ��array�ж���仯��ʱ�� diff�����������ԭʼ�����index

			fc::variants diff_json;
			for (size_t i = 0; i < a_array.size(); i++)
			{
				if (i >= b_array.size())
				{
					// ɾ��Ԫ��
					fc::variants item_diff;
					item_diff.push_back("-");
					item_diff.push_back((int)i);
					item_diff.push_back(a_array[i]);
					diff_json.push_back(item_diff);
				}
				else
				{
					auto item_value_diff = diff(a_array[i], b_array[i]);
					if (item_value_diff->is_undefined()) // û�з����ı�
						continue;
					// �޸�Ԫ��
					fc::variants item_diff;
					item_diff.push_back("~");
					item_diff.push_back((int)i);
					item_diff.push_back(item_value_diff->value());
					diff_json.push_back(item_diff);
				}
			}
			for (size_t i = a_array.size(); i < b_array.size(); i++)
			{
				// ��������old���Ǵ�����new��
				fc::variants item_diff;
				item_diff.push_back("+");
				item_diff.push_back((int)i);
				item_diff.push_back(b_array[i]);
				diff_json.push_back(item_diff);
			}
			if (diff_json.size() < 1)
			{
				return DiffResult::make_undefined_diff_result();
			}
			return std::make_shared<DiffResult>(diff_json);
		}
		else
		{
			throw JsonDiffException(std::string("not supported json value type to diff ") + json_dumps(old_json));
		}
	}

	JsonValue JsonDiff::patch_by_string(const std::string& old_json_value, DiffResultP diff_info)
	{
		return patch(json_loads(old_json_value), diff_info);
	}

	JsonValue JsonDiff::patch(const JsonValue& old_json, const DiffResultP& diff_info)
	{
		auto old_json_type = guess_json_value_type(old_json);
		auto diff_json = diff_info->value();
		auto result = json_deep_clone(old_json);
		if (diff_info->is_undefined() || diff_json.is_null())
			return result;
		
		auto old_json_str = json_dumps(old_json);
		auto diff_json_str = json_dumps(diff_json);
		
		if (is_scalar_json_value_type(old_json_type) || is_scalar_value_diff_format(diff_json))
		{ // TODO: ����ж�Ҫ�޸ĵü�׼ȷһ�㣬�޸�diffjson��ʽ������{__old: ..., __new: ...}����ͨobject diff
			if (!diff_json.is_object())
				throw JsonDiffException("wrong format of diffjson of scalar json value");
			result = diff_json[JSONDIFF_KEY_NEW_VALUE];
		}
		else if (old_json_type == JsonValueType::JVT_OBJECT)
		{
			auto old_json_obj = old_json.as<fc::mutable_variant_object>();
			auto diff_json_obj = diff_json.as<fc::mutable_variant_object>();
			auto result_obj = result.as<fc::mutable_variant_object>();
			for (auto i = diff_json_obj.begin(); i != diff_json_obj.end(); i++)
			{
				auto key = i->key();
				auto diff_item = i->value();
				// ���key�� <key>__deleted ���� <key>__added������ɾ���������ӣ��������޸�����key��ֵ
				if (utils::string_ends_with(key, JSONDIFF_KEY_DELETED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_DELETED_POSTFIX))
				{
					auto old_key = utils::string_without_ext(key, JSONDIFF_KEY_DELETED_POSTFIX);
					if (old_json_obj.find(old_key) != old_json_obj.end())
					{
						// ��ɾ�����Բ���
						result_obj.erase(old_key);
						continue;
					}
				}
				else if (utils::string_ends_with(key, JSONDIFF_KEY_ADDED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_ADDED_POSTFIX))
				{
					auto old_key = utils::string_without_ext(key, JSONDIFF_KEY_ADDED_POSTFIX);
					// ���������Բ���
					result_obj[old_key] = diff_item;
					continue;
				}
				// �������޸�����key��ֵ
				if (old_json_obj.find(key) == old_json_obj.end())
					throw JsonDiffException("wrong format of diffjson of this old version json");
				auto sub_item_new = patch(old_json_obj[key], std::make_shared<DiffResult>(diff_item));
				result_obj[key] = sub_item_new;
			}
			return result_obj;
		}
		else if (old_json_type == JsonValueType::JVT_ARRAY)
		{
			auto old_json_array = old_json.as<fc::variants>();
			auto diff_json_array = diff_json.as<fc::variants>();
			auto result_array = result.as<fc::variants>();
			for (size_t i = 0; i < diff_json_array.size(); i++)
			{
				if (!diff_json_array[i].is_array())
					throw JsonDiffException("diffjson format error for array diff");
				auto diff_item = diff_json_array[i].as<fc::variants>();
				if (diff_item.size() != 3)
					throw JsonDiffException("diffjson format error for array diff");
				auto op_item = diff_item[0].as_string();
				auto pos = diff_item[1].as_uint64();
				auto inner_diff_json = diff_item[2];
				// FIXME�� һ��array�ж���仯��ʱ�� diff�����������ԭʼ�����index����������Ӧ���ҳ� pos => old_json��ֵͬ��pos
				if (op_item == std::string("+"))
				{
					// ����Ԫ��
					result_array.insert(result_array.begin() + pos, inner_diff_json);
				}
				else if (op_item == std::string("-"))
				{
					// ɾ��Ԫ��
					result_array.erase(result_array.begin() + pos);
				}
				else if (op_item == std::string("~"))
				{
					// �޸�Ԫ��
					auto sub_item_new = patch(old_json_array[i], std::make_shared<DiffResult>(inner_diff_json));
					result_array[pos] = sub_item_new;
				}
				else
				{
					throw JsonDiffException(std::string("not supported diff array op now: ") + op_item);
				}
			}
			return result_array;
		}
		else
		{
			throw JsonDiffException(std::string("not supported json value type to merge patch ") + json_dumps(old_json));
		}
		return result;
	}

	JsonValue JsonDiff::rollback_by_string(const std::string& new_json_value, DiffResultP diff_info)
	{
		return rollback(json_loads(new_json_value), diff_info);
	}

	JsonValue JsonDiff::rollback(const JsonValue& new_json, DiffResultP diff_info)
	{
		auto new_json_type = guess_json_value_type(new_json);
		auto diff_json = diff_info->value();
		auto result = json_deep_clone(new_json);
		if (diff_info->is_undefined() || diff_json.is_null())
			return result;

		auto new_json_str = json_dumps(new_json);
		auto diff_json_str = json_dumps(diff_json);

		if (is_scalar_json_value_type(new_json_type) || is_scalar_value_diff_format(diff_json))
		{ // TODO: ����ж�Ҫ�޸ĵü�׼ȷһ�㣬�޸�diffjson��ʽ������{__old: ..., __new: ...}����ͨobject diff
			if (!diff_json.is_object())
				throw JsonDiffException("wrong format of diffjson of scalar json value");
			result = diff_json[JSONDIFF_KEY_OLD_VALUE];
		}
		else if (new_json_type == JsonValueType::JVT_OBJECT)
		{
			auto new_json_obj = new_json.as<fc::mutable_variant_object>();
			auto diff_json_obj = diff_json.as<fc::mutable_variant_object>();
			auto result_obj = result.as<fc::mutable_variant_object>();
			for (auto i = diff_json_obj.begin(); i != diff_json_obj.end(); i++)
			{
				auto key = i->key();
				auto diff_item = i->value();
				// ���key�� <key>__deleted ���� <key>__added������ɾ���������ӣ��������޸�����key��ֵ
				if (utils::string_ends_with(key, JSONDIFF_KEY_ADDED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_ADDED_POSTFIX))
				{
					auto origin_key = utils::string_without_ext(key, JSONDIFF_KEY_ADDED_POSTFIX);
					if (new_json_obj.find(origin_key) != new_json_obj.end())
					{
						// ���������Բ�������Ҫ�ع�
						result_obj.erase(origin_key);
						continue;
					}
				}
				else if (utils::string_ends_with(key, JSONDIFF_KEY_DELETED_POSTFIX) && key.size() > strlen(JSONDIFF_KEY_DELETED_POSTFIX))
				{
					auto origin_key = utils::string_without_ext(key, JSONDIFF_KEY_DELETED_POSTFIX);
					// ��ɾ�����Բ�������Ҫ�ع�
					result_obj[origin_key] = diff_item;
					continue;
				}
				// �������޸�����key��ֵ
				if (new_json_obj.find(key) == new_json_obj.end())
					throw JsonDiffException("wrong format of diffjson of this old version json");
				auto sub_item_new = rollback(new_json_obj[key], std::make_shared<DiffResult>(diff_item));
				result_obj[key] = sub_item_new;
			}
			return result_obj;
		}
		else if (new_json_type == JsonValueType::JVT_ARRAY)
		{
			auto new_json_array = new_json.as<fc::variants>();
			auto diff_json_array = diff_json.as<fc::variants>();
			auto result_array = result.as<fc::variants>();
			for (size_t i = 0; i < diff_json_array.size(); i++)
			{
				if (!diff_json_array[i].is_array())
					throw JsonDiffException("diffjson format error for array diff");
				auto diff_item = diff_json_array[i].as<fc::variants>();
				if (diff_item.size() != 3)
					throw JsonDiffException("diffjson format error for array diff");
				auto op_item = diff_item[0].as_string();
				auto pos = diff_item[1].as_uint64(); // pos��old��pos�� FIXME�� �¾ɶ����pos��һ��һ��
				auto inner_diff_json = diff_item[2];
				// FIXME�� һ��array�ж���仯��ʱ�� diff�����������ԭʼ�����index����������Ӧ���ҳ� pos => old_json��ֵͬ��pos
				if (op_item == std::string("-"))
				{
					// ɾ��Ԫ�أ���Ҫ�ع�
					result_array.insert(result_array.begin() + pos, inner_diff_json);
				}
				else if (op_item == std::string("+"))
				{
					// ����Ԫ�أ���Ҫ�ع�
					result_array.erase(result_array.begin() + pos);
				}
				else if (op_item == std::string("~"))
				{
					// �޸�Ԫ��
					auto sub_item_new = rollback(new_json_array[i], std::make_shared<DiffResult>(inner_diff_json));
					result_array[pos] = sub_item_new;
				}
				else
				{
					throw JsonDiffException(std::string("not supported diff array op now: ") + op_item);
				}
			}
			return result_array;
		}
		else
		{
			throw JsonDiffException(std::string("not supported json value type to rollback diff from ") + json_dumps(new_json));
		}
		return result;
	}
}
