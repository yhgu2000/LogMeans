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
   * @param[out] cata 聚类结果
   * @param[out] mseHist 误差历史
   * @param[out] ansIndex 最终结果在 \p mseHist 中的索引
   */
  void operator()(const DataSet& data,
                  Catalog* cata,
                  MseHistory* mseHist,
                  std::size_t* ansIndex);

  /**
   * @brief 二分查找版
   */
  void binary_search(const DataSet& data,
                     Catalog* cata,
                     MseHistory* mseHist,
                     std::size_t* ansIndex);

private:
  KMeans mKMeans;
};

} // namespace Lib
