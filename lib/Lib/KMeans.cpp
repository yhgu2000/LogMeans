#include "KMeans.hpp"
#include "DistinctRandom.cpp"
#define INF 0x3f3f3f3f;

namespace Lib {

void
KMeans::operator()(const DataSet& data, int k, Catalog* cata, float* mse)
{
  struct SomeInfo : public Info
  {
  public:
    std::string info() noexcept override { return "bla bla bla"; }
  };

  // 计时一次
  time("KMeans", new SomeInfo(), true);

  {
    // 计时一个作用域
    Scope scope(*this, "KMeans-Scope-1");
  }

  // TODO
  int data_nums = data.rows();
  int dims = data.cols();

  std::random_device rd;
  std::mt19937 gen(rd());
  lib::distinct_uniform_int_distribution<> dis(0,data_nums-1);

  // 聚类中心点
  // 初始化：从数据中随机选k个
  Eigen::MatrixXd centers(k,dims);
  for(int i =0;i<k;i++){
      int idx = dis(gen);
      centers.row(i) = data.row(idx);
  }

  // 迭代
  Eigen::VectorXi new_labels(data_nums);
  int step = 0;

  while(true){
      // 分类
      // 对数据集中每个点，找到最近的k_idx
      for(int i=0;i<data_nums;i++){
          float min_dist = INF;
          int min_idx = -1;
          for(int j =0;j<k;j++){
              float dist = (data.row(i)-centers.row(j)).norm();
              if(dist<min_dist){
                  min_dist = dist;
                  min_idx = j;
              }
          }
          new_labels(i) = min_idx;
      }

      // 更新聚类中心
      Eigen::MatrixXd new_centers(k,dims);
      Eigen::VectorXi count(k);

      count.setZero();
      for(int i=0;i<data_nums;i++){
          new_centers.row(new_labels(i))+=data.row(i);
          count(new_labels(i))++;
      }                
      for(int i =0;i<k;i++){
          if(count(i)==0){
              // 应对离群中心点，重新随机生成
              int idx = dis(gen);
              new_centers.row(i)=data.row(idx);
          }else{
              new_centers.row(i)/=count(i);
          }
      }

      // 收敛条件
      error = (new_centers - centers).norm();
      if(error<eps){
          break;
      }
      step++;
      centers = new_centers;
  }
  *cata = new_labels;
  *mse = error;
};

} // namespace Lib
