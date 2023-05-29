#pragma once

#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class KMeans : public Profiler
{
public:
  DataSet::value_type eps{ 1e-6 }; ///< 收敛阈值

public:
  KMeans() = default;

  KMeans(DataSet::value_type eps)
    : eps(eps)
  {
  }

  KMeans(const Profiler& prof)
    : Profiler(prof)
  {
  }

public:
  /**
   * @param[in] data 数据集
   * @param[in] k 聚类数
   * @param[out] cata 聚类结果
   * @param[out] mse 误差
   */
  void operator()(const DataSet& data,
                  int k,
                  Catalog* cata,
                  DataSet::value_type* mse);
};

} // namespace Lib
