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
  DataSet::value_type error; //每轮迭代后，中心点相比上一轮的距离差
  DataSet::value_type eps = 1e-8;   //收敛阈值
  DataSet::value_type sse;
  void operator()(const DataSet& data,
                  int k,
                  Catalog* cata,
                  DataSet::value_type* mse);

private:
  DataSet::value_type cal_sse(const int data_nums, const int k,const DataSet& data, Catalog* new_labels, const Eigen::MatrixXd& centers){
    DataSet::value_type sse = 0; 
    for (int i = 0; i < data_nums; i++) {
    double min_dist = 1e10;
    int min_idx = -1;
      for (int j = 0; j < k; j++) {
        double dist = (data.col(i) - centers.col(j)).norm();
        if (dist < min_dist) {
          min_dist = dist;
          min_idx = j;
        }
      }
      (*new_labels)(i) = min_idx;
      sse += min_dist;
    }
    return sse;
  }
};

} // namespace Lib
