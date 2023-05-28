#include "Elbow.hpp"

namespace Lib {

void
Elbow::operator()(const DataSet& data,
                  Catalog* cata,
                  MseHistory* mseHist,
                  std::size_t* ansIndex)
{
  // TODO
  mseHist->clear();

  int dims = data.rows();
  int data_nums = data.cols();

  Catalog tmp_cata;  // 存最大mse_rate对应k的分类结果
  DataSet::value_type prev_mse = 1; // 前一个k的mse，计算mse_rate用
  DataSet::value_type mse_rate = 0; // 维护最大mse_rate
  std::size_t elbow_k = 1;

  for(int k=2;k<data_nums;k++){
    DataSet::value_type mse;
    mKMeans(data, k, cata, &mse);
    DataSet::value_type tmp_mse_rate = prev_mse/mse;
    if(tmp_mse_rate>mse_rate){
      mse_rate = tmp_mse_rate;
      tmp_cata = *cata;
      elbow_k = k;
    }
    mseHist->emplace_back(k,mse);
    std::cout<<"k="<<k<<" mse="<<mse<<" mse_rate="<<tmp_mse_rate<<"\n";
    prev_mse = mse;
  }
  std::cout<<"elbow_k is"<<elbow_k<<"\n";
  *cata = tmp_cata;
  *ansIndex = elbow_k;

  /**
   * xxxIndex 变量里存的是 mseHist 中项的索引，
   * mseHist 中保存的是聚类数 k 到 mse 的映射。
   */

}

} // namespace Lib
