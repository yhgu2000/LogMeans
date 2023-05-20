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
  static _T load_bin(const char* path);

public:
  using _S::_S;

public:
  bj::value to_json();
  void dump_bin(const char* path);
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
  static _T load_bin(const char* path);

public:
  using _S::_S;

public:
  bj::value to_json();
  void dump_bin(const char* path);
};

} // namespace Lib
