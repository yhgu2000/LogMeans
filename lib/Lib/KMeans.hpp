/*
 * @Description: 说明
 * @Author: Marcel
 * @Date: 2023-05-28 15:24:48
 */
#pragma once

#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class KMeans : public Profiler
{
public:
  DataSet::value_type mEpsRatio{ 0.001 }; ///< 判断收敛的MSE变化率阈值

public:
  KMeans() = default;

  KMeans(DataSet::value_type epsRatio)
    : mEpsRatio(epsRatio)
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
