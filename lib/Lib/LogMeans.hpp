#pragma once

#include "Lib/KMeans.hpp"
#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class LogMeans : public Profiler
{
public:
  /**
   * @param[in] data 数据集
   * @param[out] cata 聚类结果
   * @param[out] mseHist 误差历史
   * @param[out] ansIndex 最终结果在 \p mseHist 中的索引
   * @param[in] minK 最小聚类数
   * @param[in] maxK 最大聚类数
   */
  void operator()(const DataSet& data,
                  Catalog* cata,
                  MseHistory* mseHist,
                  std::size_t* ansIndex,
                  int minK,
                  int maxK);

  /**
   * @brief 二分查找版
   */
  void binary_search(const DataSet& data,
                     Catalog* cata,
                     MseHistory* mseHist,
                     std::size_t* ansIndex,
                     int minK,
                     int maxK);

private:
  class KMeans : public Lib::KMeans
  {
    LogMeans& mSelf;

  public:
    KMeans(LogMeans& self)
      : Lib::KMeans(self)
      , mSelf(self)
    {
    }

    void report(Profiler::Entry& entry) noexcept override;
  } mKMeans{ *this };
};

} // namespace Lib
