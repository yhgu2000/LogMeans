#
# 测试 lib.hpp
#
add_executable(test_lib lib.cpp)

target_link_libraries(test_lib PRIVATE test_util Lib)

target_compile_definitions(test_lib PRIVATE BOOST_TEST_MODULE=lib)



#
# 测试 LogMeans 实现
#
add_subdirectory(LogMeans)
