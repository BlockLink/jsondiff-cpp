#include <iostream>
#include <cassert>
#include <jsondiff/jsondiff.h>

using namespace jsondiff;

int main()
{
	std::cout << "Hello World!" << std::endl;
	{
		JsonDiff json_diff;
		auto origin = "123";
		auto result = "{\"a\":true,\"b\":\"hello\"}";
		auto diff_result = json_diff.diff_by_string(std::string(origin), std::string(result));
		auto diff_result_str = diff_result->pretty_str();
		std::cout << diff_result_str << std::endl;
		auto patched = json_diff.patch_by_string(origin, diff_result);
		std::cout << "patched: " << json_dumps(patched) << std::endl;
		assert(json_dumps(patched) == json_dumps(json_loads(result)));
	}
	{
		JsonDiff json_diff;
		auto origin = R"({
  "foo": 42,
  "bar": 100,
  "boz": [
    1,
    2,
    3,
    4,
    5,
    6
  ],
  "array": [1,2,3,4,5,6],
  "fubar": {
    "kaboom": {
      "note": "We're running dangerously low on metasyntatic variables here",
      "afoo": {
        "abar": "raba",
        "aboz": "zoba",
        "afubar": "rabufa"
      },
      "akaboom": 200
    }
  }
})";
		auto result = R"({
  "foo": 42,
  "bar": 100,
  "boz": {
      "a":[1,2,3]
  },
  "array": [0, 1,3,4,6,7,5],
  "fubar": {
    "kaboom": {
      "note": "We're running dangerously low on metasyntatic variables here",
      "afoo": {
        "abar": "raba",
        "aboz": "zozoba",
        "afubar": "rabufa",
        "c": 123
      },
      "akaboom": 200
    }
  }
})";
		auto diff_result = json_diff.diff_by_string(std::string(origin), std::string(result));
		auto diff_result_str = diff_result->pretty_str();
		auto diff_result_pretty_str = diff_result->pretty_diff_str();
		std::cout << "diff: " << std::endl << diff_result_str << std::endl;
		std::cout << "diff pretty: " << std::endl << diff_result_pretty_str << std::endl;
		auto patched = json_diff.patch_by_string(origin, diff_result);
		std::cout << "patched: " << json_dumps(patched) << std::endl;
		assert(json_dumps(patched) == json_dumps(json_loads(result)));

		auto rollbacked = json_diff.rollback_by_string(result, diff_result);
		std::cout << "rollbacked: " << json_dumps(rollbacked) << std::endl;
		assert(json_dumps(rollbacked) == json_dumps(json_loads(origin)));
	}
	int a;
	std::cin >> a;
	return 0;
}
