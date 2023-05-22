#include "lib.hpp"
#include "CFile64.hpp"
#include <omp.h>

namespace Lib {

DataSet
DataSet::from_json(const bj::value& json)
{
  const auto& obj = json.as_object();
  auto rows = obj.at("rows").as_int64();
  auto cols = obj.at("cols").as_int64();
  const auto& dataArr = obj.at("data").as_array();

  DataSet ret(rows, cols);
  auto* p = ret.data();
  for (const auto& v : dataArr)
    *p++ = v.as_double();

  return ret;
}

bj::value
DataSet::to_json() const
{
  bj::object ret;

  ret["rows"] = this->rows();
  ret["cols"] = this->cols();

  bj::array dataArr;
  for (auto *p = data(), *end = data() + size(); p != end; ++p)
    dataArr.emplace_back(*p);

  ret["data"] = std::move(dataArr);
  return ret;
}

void
DataSet::dump_bin(const char* path) const
{
  {
    CFile64 file(path, "wb");
    CFile64::Closer closer(file);
    std::uint32_t rows = this->rows(), cols = this->cols();
    file.write(&rows, sizeof(std::uint32_t), 1);
    file.write(&cols, sizeof(std::uint32_t), 1);
  }

  std::int64_t size = this->size();
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "r+b");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) * 2 + sizeof(value_type) * begin, SEEK_SET);
    file.write(data() + begin, sizeof(value_type), tsize);
  }
}

void
DataSet::load_bin(const char* path)
{
  std::uint32_t rows, cols;
  {
    CFile64 file(path, "rb");
    CFile64::Closer closer(file);
    file.read(&rows, sizeof(std::uint32_t), 1);
    file.read(&cols, sizeof(std::uint32_t), 1);
  }
  resize(rows, cols);

  std::int64_t size = this->size();
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "rb");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) * 2 + sizeof(value_type) * begin, SEEK_SET);
    file.read(data() + begin, sizeof(value_type), tsize);
  }
}

Catalog
Catalog::from_json(const bj::value& json)
{
  const auto& arr = json.as_array();
  Catalog ret(arr.size());
  auto* p = ret.data();
  for (const auto& v : arr)
    *p++ = v.as_int64();
  return ret;
}

bj::value
Catalog::to_json() const
{
  bj::array ret;
  for (auto *p = data(), *end = data() + size(); p != end; ++p)
    ret.emplace_back(*p);
  return ret;
}

void
Catalog::dump_bin(const char* path) const
{
  std::uint32_t size = this->size();

  {
    CFile64 file(path, "wb");
    CFile64::Closer closer(file);
    file.write(&size, sizeof(std::uint32_t), 1);
  }

#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "r+b");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) + sizeof(value_type) * begin, SEEK_SET);
    file.write(data() + begin, sizeof(value_type), tsize);
  }
}

void
Catalog::load_bin(const char* path)
{
  std::uint32_t size;
  {
    CFile64 file(path, "rb");
    CFile64::Closer closer(file);
    file.read(&size, sizeof(std::uint32_t), 1);
  }
  resize(size);

#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "rb");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) + sizeof(value_type) * begin, SEEK_SET);
    file.read(data() + begin, sizeof(value_type), tsize);
  }
}

} // namespace Lib
