#pragma once

#include "Profiler.hpp"
#include "lib.hpp"
#include "DistinctRandom.cpp"

namespace Lib {

class KMeans : public Profiler
{
public:
  /**
   * @param[in] data 数据集
   * @param[in] k 聚类数
   * @param[out] cata 聚类结果
   * @param[out] mse 误差
   */
  double error; //迭代过程的mse
  double eps = 1e-6;   //收敛阈值
  void operator()(const DataSet& data,
                  int k,
                  Catalog* cata,
                  DataSet::value_type* mse);
};

} // namespace Lib
