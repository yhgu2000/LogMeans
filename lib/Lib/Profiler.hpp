#pragma once

#include "cpp"
#include <atomic>
#include <chrono>
#include <memory>
#include <ostream>

namespace Lib {
class Profiler;
}

/**
 * @brief 标准输出流打印函数，以缩进文本的方式打印输出。
 */
std::ostream&
operator<<(std::ostream& out, const Lib::Profiler& prof);

namespace Lib {

/**
 * @brief 线程安全的性能分析器。
 */
class Profiler
{
public:
  using Clock = std::chrono::high_resolution_clock;

  /**
   * @brief 用于给记录提供额外信息的接口类。
   */
  struct Info
  {
    virtual ~Info() = default;

    /**
     * @brief 获取附加信息。
     *
     * 如果计时序列会被并发访问，那么该方法必须是线程安全的。
     */
    virtual std::string info() noexcept = 0;
  };

  class Entry;
  class Iterator;
  class Scope;
  class Profiled;

public:
  Profiler();

  /**
   * @brief 该构造函数是浅拷贝，拷贝后的对象与原对象共享计时序列。
   */
  Profiler(const Profiler&) = default;

  /**
   * @brief 该赋值操作是浅拷贝，拷贝后的对象与原对象共享计时序列。
   */
  Profiler& operator=(const Profiler&) = default;

public:
  /**
   * @brief 获取 Profiler 的创建的时刻。
   */
  Clock::time_point initial() const noexcept;

  /**
   * @brief 记录一次计时。
   *
   * 该方法是线程安全的，多个线程调用同一个 Profiler 对象的该方法、或者多个线程
   * 调用指向同一计时序列的该方法都不会发生并发竞争。
   *
   * @param tag 计时标签。
   * @param info 附加信息，可为空。
   * @param owned 是否托管 info，若 info 为空则此参数必须为 false。
   *
   * @return 本次计时构造的记录条目，是引用，当心指针悬挂。
   */
  Entry& time(const char* tag,
              Info* info = nullptr,
              bool owned = true) noexcept;

public:
  ///@name 迭代器。
  ///@{
  Iterator begin() const noexcept;
  Iterator end() const noexcept;
  ///@}

private:
  std::shared_ptr<Entry> mHead;

private:
  friend std::ostream& ::operator<<(std::ostream& out, const Profiler& prof);
};

/**
 * @brief 记录条目。
 */
class Profiler::Entry
{
  friend class Profiler;

public:
  bool mOwned;             ///< 是否持有 mInfo
  Info* mInfo;             ///< 附加信息
  const char* mTag;        ///< 计时标签
  Clock::time_point mTime; ///< 计时点

public:
  Entry(const Entry&) = delete;
  Entry(Entry&&) = delete;
  Entry& operator=(const Entry&) = delete;
  Entry& operator=(Entry&&) = delete;

  ~Entry() noexcept;

private:
  std::atomic<Entry*> mNext{ nullptr };

private:
  Entry() = default;

  Entry(Clock::time_point time, const char* tag, Info* info, bool owned)
    : mOwned(owned)
    , mInfo(info)
    , mTag(tag)
    , mTime(time)
  {
  }
};

class Profiler::Iterator
{
public:
  Iterator(Entry* entry) noexcept
    : mEntry(entry)
  {
  }

public:
  Iterator& operator++() noexcept
  {
    mEntry = mEntry->mNext.load();
    return *this;
  }

  Iterator operator++(int) noexcept
  {
    Iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const Iterator& other) const noexcept
  {
    return mEntry == other.mEntry;
  }

  bool operator!=(const Iterator& other) const noexcept
  {
    return mEntry != other.mEntry;
  }

  Entry& operator*() const noexcept { return *mEntry; }

  Entry* operator->() const noexcept { return mEntry; }

private:
  Entry* mEntry;
};

/**
 * @brief 作用域计时类，在构造时进行一次计时，在析构时再自动进行一次计时
 */
class Profiler::Scope
{
public:
  struct EnterInfo : public Info
  {
    std::string info() noexcept override;
  };

  struct LeaveInfo : public Info
  {
    std::string info() noexcept override;
  };

public:
  static EnterInfo gEnterInfo; ///< 作用域进入时的附加信息
  static LeaveInfo gLeaveInfo; ///< 作用域离开时的附加信息

public:
  Scope(Profiler& self, const char* tag)
    : _(self)
    , mTag(tag)
  {
    _.time(mTag, &gEnterInfo, false);
  }

  Scope(const Scope&) = delete;
  Scope(Scope&&) = delete;
  Scope& operator=(const Scope&) = delete;
  Scope& operator=(Scope&&) = delete;

  ~Scope() { _.time(mTag, &gLeaveInfo, false); }

private:
  Profiler& _;
  const char* mTag;
};

} // namespace Lib

namespace Lib {

inline Profiler::Profiler()
  : mHead(new Entry(Clock::now(), nullptr, nullptr, false))
{
}

inline Profiler::Clock::time_point
Profiler::initial() const noexcept
{
  return mHead->mTime;
}

inline Profiler::Iterator
Profiler::begin() const noexcept
{
  return Iterator(mHead.get());
}

inline Profiler::Iterator
Profiler::end() const noexcept
{
  return Iterator(nullptr);
}

} // namespace Lib
