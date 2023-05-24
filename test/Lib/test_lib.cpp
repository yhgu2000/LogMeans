#include "util.hpp"
#include <Lib/lib.hpp>

using namespace Lib;

//==============================================================================
// 功能性测试
//==============================================================================

BOOST_AUTO_TEST_SUITE(functionality)

BOOST_AUTO_TEST_CASE(DataSet_io_json)
{
  DataSet ds(13, 17);
  for (auto *p = ds.data(), *end = ds.data() + ds.size(); p != end; ++p)
    *p = genrand::norm();
  auto json = matx_to_json(ds);
  auto ds2 = json_to_matx<DataSet::value_type>(json);
  BOOST_TEST((ds == ds2));
}

BOOST_AUTO_TEST_CASE(DataSet_io_bin)
{
  DataSet ds(32, 32);
  for (auto *p = ds.data(), *end = ds.data() + ds.size(); p != end; ++p)
    *p = genrand::norm();
  ds.dump_bin("dataset");

  DataSet ds2;
  ds2.load_bin("dataset");
  BOOST_TEST((ds == ds2));
}

BOOST_AUTO_TEST_CASE(Catalog_io_json)
  Catalog ct(7);
  for (auto *p = ct.data(), *end = ct.data() + ct.size(); p != end; ++p)
    *p = genrand::range<int>();
  auto json = matx_to_json(ct);
  auto ct2 = json_to_matx<int>(json);
  BOOST_TEST((ct == ct2));
}

BOOST_AUTO_TEST_CASE(Catalog_io_bin)
{
  Catalog ct(11);
  for (auto *p = ct.data(), *end = ct.data() + ct.size(); p != end; ++p)
    *p = genrand::range<int>();
  matx_dump_bin(ct, "catalog");

  Catalog ct2;
  matx_load_bin(&ct2, "catalog");
  BOOST_TEST((ct == ct2));
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
