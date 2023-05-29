#include "KMeans.hpp"
#include <random>
#include <string>

using namespace std::string_literals;

namespace Lib {

void
KMeans::operator()(const DataSet& data,
                   int k,
                   Catalog* cata,
                   DataSet::value_type* mse)
{
  Scope scopeKMeans(*this, "KMeans");

  int dims = data.rows();
  int dataNums = data.cols();
  std::default_random_engine gen{ std::random_device()() };

  // 初始化：从数据中随机选k个
  DataSet centers(dims, k);
  {
    for (int i = 0; i < k; ++i)
      centers.col(i) = data.col(std::uniform_int_distribution<>(
        i * dataNums / k, (i + 1) * dataNums / k)(gen));
  }

  auto& labels = *cata;
  labels.resize(dataNums);
  time("KMeans-init");

  DataSet centersNew(dims, k); // 每轮更新的k个中心点
  Eigen::VectorXi kcount(k);   // 每轮隶属某个中心点的点数量

  for (int step = 0;; step++) {
    // 分类，对数据集中每个点，找到最近的k_idx
    DataSet::value_type sse = 0;
#pragma omp parallel for reduction(+ : sse)
    for (int i = 0; i < dataNums; i++) {
      DataSet::value_type minDist =
        std::numeric_limits<DataSet::value_type>::max();
      int minIdx = -1;
      for (int j = 0; j < k; j++) {
        DataSet::value_type dist = (data.col(i) - centers.col(j)).norm();
        if (dist < minDist)
          minDist = dist, minIdx = j;
      }
      labels(i) = minIdx;
      sse += minDist;
    }
    *mse = sse / dataNums;

    // 更新聚类中心
    kcount.setZero();
    centersNew.setZero();
    for (int i = 0; i < dataNums; ++i) {
      int tmp = labels(i);
      centersNew.col(tmp) += data.col(i);
      ++kcount(tmp);
    }
    for (int i = 0; i < k; ++i) {
      if (kcount(i) == 0) {
        // 应对离群中心点，重新随机生成
        int idx = std::uniform_int_distribution<>(0, dataNums)(gen);
        centersNew.col(i) = data.col(idx);
      } else {
        centersNew.col(i) /= kcount(i);
      }
    }

    // 收敛条件
    auto delta = (centersNew - centers).norm();
    std::swap(centers, centersNew);
    if (delta < eps)
      break;

    struct IterInfo : public Profiler::Info
    {
      int mStep;
      DataSet::value_type mMse;

      IterInfo(int step, DataSet::value_type mse)
        : mStep(step)
        , mMse(mse)
      {
      }

      std::string info() noexcept override
      {
        return "MSE["s + std::to_string(mStep) + "]=" + std::to_string(mMse);
      }
    }* info = new IterInfo(step, *mse);
    time("KMeans-iter", info, true);
  }
};

} // namespace Lib
