#include "LogMeans.hpp"

namespace Lib {

void
LogMeans::operator()(const DataSet& data,
                     Catalog* cata,
                     int* k,
                     DataSet::value_type* mse)
{
  DataSet::Index lft = 1, rht = data.cols();
  DataSet::value_type lftMse, rhtMse;
  mKMeans(data, lft, cata, &lftMse);
  mKMeans(data, rht, cata, &rhtMse);

  while (rht - lft > 1) {
    auto mid = (lft + rht) / 2;
    mKMeans(data, mid, cata, mse);

    if (*mse - lft > rhtMse - *mse)
      rht = mid;
    else
      lft = mid;
  }

  *k = rht;
}

} // namespace Lib
