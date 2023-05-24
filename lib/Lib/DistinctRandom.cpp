#include <unordered_set>
#include <random>

/*返回0~k之间的不重复的随机数*/

namespace lib{

template <typename IntType = int>
class distinct_uniform_int_distribution {
 public:
  using result_type = IntType;

 private:
  using set_type    = std::unordered_set<result_type>;
  using distr_type  = std::uniform_int_distribution<result_type>;

 public:
  distinct_uniform_int_distribution(result_type inf, result_type sup) :  
    inf_(inf),
    sup_(sup),
    range_(sup_ - inf_ + 1),                                             
    distr_(inf_, sup_)
    {}
  void reset() {                                                         
    uset_.clear();
    distr_.reset();
  }

  template <typename Generator>
  result_type operator()(Generator& engine) {                            
    if (not(uset_.size() < range_)) { std::terminate(); }                
    result_type res;
    do { res = distr_(engine); } while (uset_.count(res) > 0);           
    uset_.insert(res);
    return res;
  }

  result_type min() const { return inf_; }
  result_type max() const { return sup_; }

 private:
  const result_type inf_;
  const result_type sup_;
  const size_t      range_;
  distr_type        distr_;
  set_type          uset_;
};

}

