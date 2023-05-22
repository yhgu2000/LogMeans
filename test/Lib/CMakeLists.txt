#
# 测试 lib.hpp
#
add_executable(test_lib test_lib.cpp)

target_link_libraries(test_lib PRIVATE test_util Lib)

target_compile_definitions(test_lib PRIVATE BOOST_TEST_MODULE=lib)
