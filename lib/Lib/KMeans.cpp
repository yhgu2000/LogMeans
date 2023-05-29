#include "KMeans.hpp"
#include <random>
#include <string>
#include <unordered_set>

#define INF 0x3f3f3f3f;

using namespace std::string_literals;

namespace Lib {

void
KMeans::operator()(const DataSet& data,
                   int k,
                   Catalog* cata,
                   DataSet::value_type* mse)
{
  Scope scopeKMeans(*this, "KMeans");

  DataSet::value_type error; // 迭代过程的mse

  int dims = data.rows();
  int data_nums = data.cols();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, data_nums - 1);

  // 聚类中心点
  // 初始化：从数据中随机选k个
  Eigen::MatrixXd centers(dims, k);
  std::unordered_set<int> idx_box;
  int tmp_i = 0;
  while (idx_box.size() < k) {
    int idx = dis(gen);
    if (idx_box.find(idx) == idx_box.end()) {
      idx_box.insert(idx);
      centers.col(tmp_i++) = data.col(idx);
    }
  }

  time("init");

  // 迭代
  Eigen::VectorXi new_labels(data_nums);

  int step = 0;

  while (true) {
    // 分类
    // 对数据集中每个点，找到最近的k_idx
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
      new_labels(i) = min_idx;
    }

    // 更新聚类中心
    // 每轮更新的k个中心点
    Eigen::MatrixXd new_centers(dims, k);

    // 每轮隶属某个中心点的点数量
    Eigen::VectorXi k_count(k);

    k_count.setZero();
    new_centers.setZero();
    for (int i = 0; i < data_nums; i++) {
      int tmp = new_labels(i);
      new_centers.col(tmp) += data.col(i);
      k_count(new_labels(i))++;
    }
    for (int i = 0; i < k; i++) {
      if (k_count(i) == 0) {
        // 应对离群中心点，重新随机生成
        int idx = dis(gen);
        new_centers.col(i) = data.col(idx);
      } else {
        new_centers.col(i) /= k_count(i);
      }
    }

    // 收敛条件
    error = (new_centers - centers).norm();
    if (error < eps) {
      break;
    }

    centers = new_centers;
    step++;

    struct IterInfo : public Profiler::Info
    {
      int step;
      DataSet::value_type mse;

      IterInfo(int step, DataSet::value_type mse)
        : step(step)
        , mse(mse)
      {
      }

      std::string info() noexcept override
      {
        return "step="s + std::to_string(step) + " mse=" + std::to_string(mse);
      }
    }* info = new IterInfo(step, error);
    time("iter", info, true);
  }

  *cata = new_labels;
  *mse = error;
};

} // namespace Lib
