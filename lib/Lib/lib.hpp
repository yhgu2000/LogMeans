#pragma once

#include "CFile64.hpp"
#include "cpp"
#include "err.hpp"
#include <Eigen/Dense>
#include <boost/json.hpp>
#include <omp.h>
#include <vector>

namespace Lib {

namespace bj = boost::json;

template<typename _Scalar, int _Rows = -1, int _Cols = -1>
static Eigen::Matrix<_Scalar, _Rows, _Cols>
json_to_matx(const bj::value& json)
{
  using Matx = Eigen::Matrix<_Scalar, _Rows, _Cols>;

  const auto& obj = json.as_object();
  auto rows = obj.at("rows").as_int64();
  auto cols = obj.at("cols").as_int64();
  const auto& dataArr = obj.at("data").as_array();

  Matx matx;
  matx.resize(rows, cols);
  if (dataArr.size() != matx.size())
    throw err::Lit("incorrect data length for JSON matrix.");

  auto* p = matx.data();
  for (const auto& v : dataArr)
    *p++ = v.to_number<_Scalar>();

  return matx;
}

/**
 * @brief 将数据集转换为 JSON 对象。
 *
 * @return bj::value 格式为：
 *
 * ```json
 * {
 *   "rows": 3,
 *   "cols": 2,
 *   "data": [ 1, 2, 3, 4, 5, 6 ] // **按列存储**的一维数组。
 * }
 * ```
 */
template<typename _Scalar, int _Rows, int _Cols>
bj::value
matx_to_json(const Eigen::Matrix<_Scalar, _Rows, _Cols>& matx)
{
  bj::object ret;

  ret["rows"] = matx.rows();
  ret["cols"] = matx.cols();

  bj::array dataArr;
  for (auto *p = matx.data(), *end = matx.data() + matx.size(); p != end; ++p)
    dataArr.emplace_back(*p);

  ret["data"] = std::move(dataArr);
  return ret;
}

/**
 * @brief 将数据集以二进制格式保存到文件，使用多线程并行加速。
 *
 * @param path 文件路径
 */
template<typename _Scalar, int _Rows, int _Cols>
void
matx_dump_bin(const Eigen::Matrix<_Scalar, _Rows, _Cols>& matx,
              const char* path)
{
  {
    CFile64 file(path, "wb");
    CFile64::Closer closer(file);
    std::uint32_t rows = matx.rows(), cols = matx.cols();
    file.write(&rows, sizeof(std::uint32_t), 1);
    file.write(&cols, sizeof(std::uint32_t), 1);
  }

  std::int64_t size = matx.size();
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "r+b");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) * 2 + sizeof(_Scalar) * begin, SEEK_SET);
    file.write(matx.data() + begin, sizeof(_Scalar), tsize);
  }
}

/**
 * @brief 从二进制文件中加载数据集，使用多线程并行加速。
 *
 * @param path 文件路径
 */
template<typename _Scalar, int _Rows, int _Cols>
void
matx_load_bin(Eigen::Matrix<_Scalar, _Rows, _Cols>* matx, const char* path)
{
  std::uint32_t rows, cols;
  {
    CFile64 file(path, "rb");
    CFile64::Closer closer(file);
    file.read(&rows, sizeof(std::uint32_t), 1);
    file.read(&cols, sizeof(std::uint32_t), 1);
  }
  matx->resize(rows, cols);

  std::int64_t size = matx->size();
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto tnum = omp_get_num_threads();

    auto begin = size * tid / tnum;
    auto tsize = size * (tid + 1) / tnum - begin;

    CFile64 file(path, "rb");
    CFile64::Closer closer(file);

    file.seek(sizeof(std::uint32_t) * 2 + sizeof(_Scalar) * begin, SEEK_SET);
    file.read(matx->data() + begin, sizeof(_Scalar), tsize);
  }
}

/**
 * @brief 数据集，每列是一个数据点，列号为 ID。
 */
using DataSet = Eigen::MatrixXd;

/**
 * @brief 聚类结果，一个列向量，每行的整数是数据集对应列的类别号。
 */
using Catalog = Eigen::VectorXi;

/**
 * @brief 误差历史，两列向量，第一列是聚类数，第二列是对应的误差。
 */
using MseHistory =
  std::vector<std::pair<Catalog::value_type, DataSet::value_type>>;

} // namespace Lib
