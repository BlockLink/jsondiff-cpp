cmake_minimum_required(VERSION 3.5)
project(jsondiff_cpp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        jsondiff-cpp/jsondiff/diff_result.cpp
        jsondiff-cpp/jsondiff/helper.cpp
        jsondiff-cpp/jsondiff/json_value_types.cpp
        jsondiff-cpp/jsondiff/jsondiff.cpp
        # jsondiff-cpp-runner/main.cpp
)

include_directories(jsondiff-cpp/include ../fc/include /usr/local/lib/boost/include)
link_directories(../fc /usr/local/lib/boost/lib)
link_libraries(fc boost_system boost_system boost_chrono boost_date_time boost_coroutine boost_context boost_thread boost_filesystem pthread)

add_library(jsondiff_cpp ${SOURCE_FILES})
add_executable(jsondiff_cpp_runner jsondiff-cpp-runner/main.cpp)
target_link_libraries(jsondiff_cpp_runner jsondiff_cpp)
