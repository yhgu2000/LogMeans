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
  MseHistory mseHist;
  std::size_t ansIndex;
  logMeans(data, &cata, &mseHist, &ansIndex);

  BOOST_TEST_MESSAGE("k = " << mseHist[ansIndex].first
                            << "\nmse = " << mseHist[ansIndex].second);
  BOOST_TEST_MESSAGE(mseHist.to_json());
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
