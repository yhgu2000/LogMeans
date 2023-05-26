#pragma once

#include "Profiler.hpp"
#include "lib.hpp"

namespace Lib {

class Elbow : public Profiler
{
public:
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
};

} // namespace Lib
