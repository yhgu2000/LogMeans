override_glob(Lib_src
  ${CMAKE_SOURCE_DIR}/lib/Lib ${CMAKE_CURRENT_SOURCE_DIR}/Lib
  EXPRESSIONS *
)

add_executable(test_log_means test.cpp ${Lib_src})

target_include_directories(test_log_means PRIVATE . ${CMAKE_SOURCE_DIR}/lib)

target_link_libraries(test_log_means PRIVATE test_util
  Eigen3::Eigen Boost::json OpenMP::OpenMP_CXX
)

target_compile_definitions(test_log_means PRIVATE BOOST_TEST_MODULE=log_means)
