#
# 测试 lib.hpp
#
add_executable(test_lib lib.cpp)

target_link_libraries(test_lib PRIVATE test_util Lib)

target_compile_definitions(test_lib PRIVATE BOOST_TEST_MODULE=lib)



#
# 测试 LogMeans 实现
#
override_glob(Lib_src
  ${CMAKE_SOURCE_DIR}/lib/Lib ${CMAKE_CURRENT_SOURCE_DIR}/LogMeans/Lib
  EXPRESSIONS *
)

add_executable(test_LogMeans LogMeans/test.cpp ${Lib_src})

target_include_directories(test_LogMeans
  PRIVATE ${CMAKE_SOURCE_DIR}/lib LogMeans
)

target_link_libraries(test_LogMeans PRIVATE test_util
  Eigen3::Eigen Boost::json OpenMP::OpenMP_CXX
)

target_compile_definitions(test_LogMeans PRIVATE BOOST_TEST_MODULE=LogMeans)
