#pragma once

#include "cpp"
#include <Eigen/Dense>
#include <boost/json.hpp>

namespace Lib {

namespace bj = boost::json;

/**
 * @brief 数据集，每列是一个数据点，列号为 ID。
 */
class DataSet : public Eigen::MatrixXd
{
  using _T = DataSet;
  using _S = Eigen::MatrixXd;

public:
  static _T from_json(const bj::value& json);

public:
  using _S::_S;

public:
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
  bj::value to_json() const;

  /**
   * @brief 将数据集以二进制格式保存到文件，使用多线程并行加速。
   *
   * @param path 文件路径
   */
  void dump_bin(const char* path) const;

  /**
   * @brief 从二进制文件中加载数据集，使用多线程并行加速。
   *
   * @param path 文件路径
   */
  void load_bin(const char* path);
};

/**
 * @brief 聚类结果，一个列向量，每行的整数是数据集对应列的类别号。
 */
class Catalog : public Eigen::VectorXi
{
  using _T = Catalog;
  using _S = Eigen::VectorXi;

public:
  static _T from_json(const bj::value& json);

public:
  using _S::_S;

public:
  /**
   * @brief 将聚类结果转换为 JSON 对象。
   *
   * @return bj::value 格式就是一个数组，每个元素都是一个 JSON 数字。
   */
  bj::value to_json() const;

  /**
   * @brief 以二进制格式保存到文件，使用多线程并行加速。
   *
   * @param path 文件路径
   */
  void dump_bin(const char* path) const;

  /**
   * @brief 加载二进制文件数据，使用多线程并行加速。
   * 
   * @param path 文件路径
   */
  void load_bin(const char* path);
};

} // namespace Lib
