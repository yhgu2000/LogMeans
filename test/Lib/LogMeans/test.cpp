#include "util.hpp"

#include "Lib/LogMeans.hpp"

using namespace Lib;

//==============================================================================
// 功能性测试
//==============================================================================

BOOST_AUTO_TEST_SUITE(functionality)

BOOST_AUTO_TEST_CASE(case0)
{
  LogMeans logMeans;

  DataSet data;
  data.setRandom(10, 100);

  Catalog cata;
  int k;
  DataSet::value_type mse;
  logMeans(data, &cata, &k, &mse);

  BOOST_TEST_MESSAGE("k = " << k << "\nmse = " << mse);
}

BOOST_AUTO_TEST_SUITE_END()

//==============================================================================
// 稳定性测试
//==============================================================================

BOOST_AUTO_TEST_SUITE(stablity)

BOOST_AUTO_TEST_SUITE_END()

//==============================================================================
// 健壮性测试
//==============================================================================

BOOST_AUTO_TEST_SUITE(robustness)

BOOST_AUTO_TEST_SUITE_END()
