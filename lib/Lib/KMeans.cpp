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
  static thread_local std::default_random_engine sRand{
    std::random_device()()
  };

  Scope scopeKMeans(*this, "KMeans");

  int dims = data.rows();
  int dataNums = data.cols();

  // 初始化：从数据中随机选k个
  DataSet centers(dims, k);
  for (std::uint64_t i = 0; i < k; ++i) {
    auto x = std::uniform_int_distribution<>(i * dataNums / k,
                                             (i + 1) * dataNums / k - 1)(sRand);
    centers.col(i) = data.col(x);
  }
  time("KMeans-init");

  auto& labels = *cata;
  labels.resize(dataNums);
  Eigen::VectorXi kcount(k); // 每轮隶属某个中心点的点数量
  DataSet::value_type mseLast = 1e-9;
  for (int step = 0;; step++) {
    // 分类，对数据集中每个点，找到最近的k_idx
    DataSet::value_type sse = 0;
#pragma omp parallel for reduction(+ : sse)
    for (int i = 0; i < dataNums; ++i) {
      DataSet::value_type minDist =
        std::numeric_limits<DataSet::value_type>::max();
      int minIdx = -1;
      for (int j = 0; j < k; j++) {
        DataSet::value_type dist = (data.col(i) - centers.col(j)).norm();
        if (dist < minDist)
          minDist = dist, minIdx = j;
      }
      labels(i) = minIdx;
      sse += minDist / dataNums; // 在加之前先除，防止数据过大而溢出
    }
    *mse = sse;

    // 更新聚类中心
    centers.setZero();
    kcount.setZero();
#pragma omp parallel for
    for (int i = 0; i < dataNums; ++i) {
      int tmp = labels(i);
      centers.col(tmp) += data.col(i);
      ++kcount(tmp);
    }
    for (int i = 0; i < k; ++i) {
      if (kcount(i) == 0) {
        // 应对离群中心点，重新随机生成
        int idx = std::uniform_int_distribution<>(0, dataNums - 1)(sRand);
        centers.col(i) = data.col(idx);
      } else {
        centers.col(i) /= kcount(i);
      }
    }

    // 收敛条件
    if (std::abs((*mse - mseLast) / mseLast) < mEpsRatio)
      break;
    mseLast = (mseLast + *mse) / 2; // 平滑

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
