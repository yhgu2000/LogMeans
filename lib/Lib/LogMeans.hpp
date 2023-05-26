#pragma once

#include "Lib/KMeans.hpp"
#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class LogMeans : public Profiler
{
public:
  LogMeans()
    : mKMeans{ *this }
  {
  }

  /**
   * @param[in] data 数据集
   * @param[out] k 聚类数
   * @param[out] cata 聚类结果
   * @param[out] mse 误差
   */
  void operator()(const DataSet& data,
                  Catalog* cata,
                  int* k,
                  DataSet::value_type* mse);

private:
  KMeans mKMeans;
};

} // namespace Lib
