#include "KMeans.hpp"

namespace Lib {

void
KMeans::operator()(const DataSet& data,
                   int k,
                   Catalog* cata,
                   DataSet::value_type* mse)
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
}

} // namespace Lib
