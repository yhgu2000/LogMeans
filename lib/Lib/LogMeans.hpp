#pragma once

#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class LogMeans : public Profiler
{
  /**
   * @param[in] data 数据集
   * @param[out] k 聚类数
   * @param[out] cata 聚类结果
   * @param[out] mse 误差
   */
  void operator()(const DataSet& data, Catalog* cata, int* k, float* mse);
};

} // namespace Lib
