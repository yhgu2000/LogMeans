#include "lib.hpp"
#include "CFile64.hpp"
#include <omp.h>

namespace Lib {

bj::value
MseHistory::to_json() const
{
  bj::array arr;
  for (const auto& p : *this) {
    bj::array row;
    row.emplace_back(p.first);
    row.emplace_back(p.second);
    arr.emplace_back(std::move(row));
  }
  return std::move(arr);
}

} // namespace Lib
