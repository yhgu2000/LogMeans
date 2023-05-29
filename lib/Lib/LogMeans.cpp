#include "LogMeans.hpp"

#include <cassert>
#include <map>
#include <tuple>
#include <vector>
#include <iostream>

#define self (*this)

namespace Lib {

namespace {

struct HeapEntry
{
  std::size_t mL, mR;
};

/**
 * @brief 算法专用的大顶堆类。
 */
struct Heap : public std::vector<HeapEntry>
{
  using _T = Heap;
  using _S = std::vector<HeapEntry>;

  MseHistory& mMseHist;

  Heap(MseHistory& mseHist)
    : _S(1) // 让下标从 1 开始
    , mMseHist{ mseHist }
  {
  }

  bool lt(const HeapEntry& lhs, const HeapEntry& rhs) const
  {
    return (mMseHist[lhs.mR].second / mMseHist[lhs.mL].second) <
           (mMseHist[rhs.mR].second / mMseHist[rhs.mL].second);
  }

  bool gt(const HeapEntry& lhs, const HeapEntry& rhs) const
  {
    return (mMseHist[lhs.mR].second / mMseHist[lhs.mL].second) >
           (mMseHist[rhs.mR].second / mMseHist[rhs.mL].second);
  }

  // void heapify();

  void heap_push(HeapEntry ent);

  HeapEntry heap_pop();
};

// void
// Heap::heapify()
// {
//   auto i = size() >> 1;
//   if ((size() & 1) != 0) {
//     auto& parent = self[i];
//     auto& child = self[i << 1];
//     if (child > parent)
//       std::swap(child, parent);
//     --i;
//   }

//   while (i != 0) {
//     auto& parent = self[i];
//     auto* child = &self[i << 1];

//     if (*(child + 1) > *child)
//       ++child;
//     if (*child > parent)
//       std::swap(*child, parent);

//     --i;
//   }
// }

void
Heap::heap_push(HeapEntry ent)
{
  auto i = size(); // 插入元素的索引
  emplace_back(std::move(ent));
  if ((size() & 1) != 0) {
    auto& parent = self[i >> 1];
    auto& child = self[i];
    if (gt(child, parent))
      std::swap(child, parent);
    i >>= 1;
  }

  while ((i >>= 1) != 0) {
    auto& parent = self[i];
    auto* child = &self[i << 1];

    if (gt(*(child + 1), *child))
      ++child;
    if (gt(*child, parent))
      std::swap(*child, parent);
  }
}

HeapEntry
Heap::heap_pop()
{
  assert(size() > 1);

  auto ret = std::move(self[1]);

  auto i = 1;
  auto* parent = &self[i];
  while ((i <<= 1) < size()) {
    auto& lchild = self[i];

    if (i + 1 < size()) {
      auto& rchild = self[i + 1];

      if (gt(lchild, rchild)) {
        *parent = std::move(lchild);
        parent = &lchild;
      }

      else {
        *parent = std::move(rchild);
        parent = &rchild;
        ++i;
      }
    }

    else {
      *parent = std::move(lchild);
      break;
    }
  }

  pop_back();
  return ret;
}

}

void
LogMeans::operator()(const DataSet& data,
                     Catalog* cata,
                     MseHistory* mseHist,
                     std::size_t* ansIndex,
                     int minK,
                     int maxK)
{
  Profiler::Scope scopeProf(*this, "LogMeans");

  mseHist->clear();

  std::map<int, std::size_t> cache;

  /**
   * xxxIndex 变量里存的是 mseHist 中项的索引，
   * mseHist 中保存的是聚类数 k 到 mse 的映射。
   */

  std::cout << minK << " " << maxK << std::endl;
  std::size_t lftIndex = 0;
  cache.emplace(minK, 0);
  {
    DataSet::value_type mse;
    mKMeans(data, minK, cata, &mse);
    mseHist->emplace_back(minK, mse);
  }

  std::size_t rhtIndex = 1;
  cache.emplace(maxK, 1);
  {
    DataSet::value_type mse;
    mKMeans(data, maxK, cata, &mse);
    mseHist->emplace_back(maxK, mse);
  }

  time("LogMeans-iterstart");

  auto& hist = *mseHist;
  Heap heap(hist);
  while (hist[rhtIndex].first - hist[lftIndex].first > 1) {
    auto mid = (hist[rhtIndex].first + hist[lftIndex].first) / 2;
    std::size_t midIndex;

    auto it = cache.find(mid);
    if (it != cache.end())
      midIndex = it->second;

    else {
      midIndex = mseHist->size();
      cache.emplace_hint(it, mid, midIndex);

      DataSet::value_type mse;
      mKMeans(data, mid, cata, &mse);
      mseHist->emplace_back(mid, mse);
    }

    heap.heap_push({ lftIndex, midIndex });
    heap.heap_push({ midIndex, rhtIndex });

    auto top = heap.heap_pop();
    lftIndex = top.mL, rhtIndex = top.mR;

    time("LogMeans-iter");
  }

  *ansIndex = rhtIndex;
}

void
LogMeans::binary_search(const DataSet& data,
                        Catalog* cata,
                        MseHistory* mseHist,
                        std::size_t* ansIndex,
                        int minK,
                        int maxK)
{
  Profiler::Scope scopeProf(*this, "LogMeans.bs");

  mseHist->clear();

  int lft = minK, rht = maxK;
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

    if (midMse / lftMse > rhtMse / midMse)
      rht = mid, rhtMse = midMse;
    else
      lft = mid, lftMse = midMse;

    time("LogMeans.bs-iter");
  }

  *ansIndex = mseHist->size() - 1;
}

void
LogMeans::KMeans::report(Profiler::Entry& entry) noexcept
{
  mSelf.report(entry);
}

} // namespace Lib
