#include "LogMeans.hpp"

#include <cassert>
#include <tuple>
#include <vector>

#define self (*this)

namespace Lib {

namespace {

struct HeapEntry
{
  int mL, mR;
  DataSet::value_type mLmse, mRmse;
};

inline bool
operator<(const HeapEntry& lhs, const HeapEntry& rhs)
{
  return (lhs.mRmse / lhs.mLmse) < (rhs.mRmse / rhs.mLmse);
}

inline bool
operator>(const HeapEntry& lhs, const HeapEntry& rhs)
{
  return (lhs.mRmse / lhs.mLmse) > (rhs.mRmse / rhs.mLmse);
}

/// @brief 应该算是个大顶堆
struct Heap : public std::vector<HeapEntry>
{
  using _T = Heap;
  using _S = std::vector<HeapEntry>;

  Heap()
    : _S(1) // 让下标从 1 开始
  {
  }

  void heapify();

  void heap_push(HeapEntry ent);

  HeapEntry heap_pop();
};

void
Heap::heapify()
{
  auto i = size() >> 1;
  if ((size() & 1) != 0) {
    auto& parent = self[i];
    auto& child = self[i << 1];
    if (child > parent)
      std::swap(child, parent);
    --i;
  }

  while (i != 0) {
    auto& parent = self[i];
    auto* child = &self[i << 1];

    if (*(child + 1) > *child)
      ++child;
    if (*child > parent)
      std::swap(*child, parent);

    --i;
  }
}

void
Heap::heap_push(HeapEntry ent)
{
  auto i = size(); // 插入元素的索引
  emplace_back(std::move(ent));
  if ((size() & 1) != 0) {
    auto& parent = self[i >> 1];
    auto& child = self[i];
    if (child > parent)
      std::swap(child, parent);
    i >>= 1;
  }

  while ((i >>= 1) != 0) {
    auto& parent = self[i];
    auto* child = &self[i << 1];

    if (*(child + 1) > *child)
      ++child;
    if (*child > parent)
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

      if (lchild > rchild) {
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
                     std::size_t* ansIndex)
{
  mseHist->clear();

  HeapEntry ent = { 1, int(data.cols()) };

  mKMeans(data, ent.mL, cata, &ent.mLmse);
  mseHist->emplace_back(ent.mL, ent.mLmse);

  mKMeans(data, ent.mR, cata, &ent.mRmse);
  mseHist->emplace_back(ent.mR, ent.mRmse);

  Heap heap;
  while (ent.mR - ent.mL > 1) {
    auto mid = (ent.mL + ent.mR) / 2;
    DataSet::value_type midMse;

    mKMeans(data, mid, cata, &midMse);
    mseHist->emplace_back(mid, midMse);

    heap.heap_push({ ent.mL, mid, ent.mLmse, midMse });
    heap.heap_push({ mid, ent.mR, midMse, ent.mRmse });

    ent = heap.heap_pop();
  }

  *ansIndex = mseHist->size() - 1;
}

void
LogMeans::binary_search(const DataSet& data,
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

    if (midMse / lftMse > rhtMse / midMse)
      rht = mid, rhtMse = midMse;
    else
      lft = mid, lftMse = midMse;
  }

  *ansIndex = mseHist->size() - 1;
}

} // namespace Lib
