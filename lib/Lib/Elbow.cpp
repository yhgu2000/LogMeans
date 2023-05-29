#include "Elbow.hpp"

namespace Lib {

void
Elbow::operator()(const DataSet& data,
                  Catalog* cata,
                  MseHistory* mseHist,
                  std::size_t* ansIndex,
                  int minK,
                  int maxK)
{
  Profiler::Scope scopeProf(*this, "Elbow");

  mseHist->clear();

  int dims = data.rows();
  int dataNums = data.cols();

  Catalog tmpCata;                 // 存最大mse_rate对应k的分类结果
  DataSet::value_type prevMse = 1; // 前一个k的mse，计算mse_rate用
  DataSet::value_type mseRate = 0; // 维护最大mseRate
  std::size_t elbowK = 1;

  for (int k = minK; k <= maxK; k++) {
    DataSet::value_type mse;
    mKMeans(data, k, cata, &mse);
    DataSet::value_type tmp_mseRate = prevMse / mse;
    if (tmp_mseRate > mseRate) {
      mseRate = tmp_mseRate;
      tmpCata = *cata;
      elbowK = k;
    }
    mseHist->emplace_back(k, mse);
    prevMse = mse;

    time("Elbow-iter");
  }

  *cata = tmpCata;
  *ansIndex = elbowK - 2; // 最大mse_rate对应k的"索引"
}

void
Elbow::KMeans::report(Profiler::Entry& entry) noexcept
{
  mSelf.report(entry);
}

} // namespace Lib
