#include "LogMeans.hpp"

namespace Lib {

void
LogMeans::operator()(const DataSet& data,
                     Catalog* cata,
                     MseHistory* mseHist,
                     std::size_t* ansIndex)
{
  mseHist->clear();

  int lft = 1, rht = data.cols();
  DataSet::value_type lftMse, rhtMse;

  mKMeans(data, lft, cata, &lftMse);
  mseHist->emplace_back(lft, lftMse);

  mKMeans(data, rht, cata, &rhtMse);
  mseHist->emplace_back(rht, rhtMse);

  while (rht - lft > 1) {
    auto mid = (lft + rht) / 2;
    DataSet::value_type midMse;

    mKMeans(data, mid, cata, &midMse);
    mseHist->emplace_back(mid, midMse);

    if (midMse - lftMse > rhtMse - midMse)
      rht = mid, rhtMse = midMse;
    else
      lft = mid, lftMse = midMse;
  }

  *ansIndex = mseHist->size() - 1;
}

void
LogMeans::modified(const DataSet& data,
                   Catalog* cata,
                   MseHistory* mseHist,
                   std::size_t* ansIndex)
{
  // TODO
}

} // namespace Lib
