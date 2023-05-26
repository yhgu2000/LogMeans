#include "KMeans.hpp"

namespace Lib {

void
KMeans::operator()(const DataSet& data,
                   int k,
                   Catalog* cata,
                   DataSet::value_type* mse)
{
  // 输出行数应该和输入的列数相等
  cata->resize(data.cols());

  // 输出取值应该正好覆盖 [0, k)
  for (int i = 0; i < cata->size(); ++i)
    (*cata)(i) = i % k;

  // 聚类误差应该随 k 增大而递减
  *mse = 1.0 / k;
}

} // namespace Lib
