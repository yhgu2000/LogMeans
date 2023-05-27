#include "Profiler.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <ostream>
#include <vector>

namespace sc = std::chrono;

template<typename T1, typename T2>
static std::ostream&
operator<<(std::ostream& out, const sc::duration<T1, T2>& dura)
{
  auto count = sc::duration_cast<sc::nanoseconds>(dura).count();

  if (count < 10000)
    out << count << "ns";
  else if ((count /= 1000) < 10000)
    out << count << "us";
  else if ((count /= 1000) < 10000)
    out << count << "ms";
  else
    out << count / 1000 << "s";

  return out;
}

std::ostream&
operator<<(std::ostream& out, const Lib::Profiler& prof)
{
  using namespace Lib;

  std::vector<Profiler::Entry*> stack;
  auto last = prof.initial();
  for (auto&& i : prof) {
    for (auto i = stack.size(); i > 1; --i)
      out << '\t';

    if (i.mInfo == &Profiler::Scope::gLeaveInfo && !stack.empty() &&
        stack.back()->mTag == i.mTag) {
      out << i.mTag << " [" << (i.mTime - stack.back()->mTime) << ']';
      stack.pop_back();
    }

    else {
      if (!stack.empty())
        out << '\t';
      out << i.mTag << " [" << (i.mTime - last) << ']';
      if (i.mInfo) {
        if (i.mInfo == &Profiler::Scope::gEnterInfo)
          stack.push_back(&i);
        else
          out << " : " << i.mInfo->info();
      }
      out << '\n';
    }

    last = i.mTime;
  }

  return out;
}

namespace Lib {

Profiler
Profiler::from_json(const bj::value& json,
                    std::set<std::string>& tags) noexcept(false)
{
  const auto& arr = json.as_array();

  Profiler prof;
  for (auto&& i : arr) {
    const auto& ent = i.as_array();

    const auto* tag =
      tags.emplace(ent.at(0).as_string().c_str()).first->c_str();

    auto time = prof.mHead->mTime +
                sc::duration_cast<Clock::duration>(
                  sc::duration<double, std::nano>(ent.at(1).as_double()));

    Info* info;
    if (ent.size() <= 2)
      info = nullptr;
    else
      info = new StrInfo(ent.at(2).as_string().c_str());

    prof.mHead->mNext.store(
      new Entry(time,
                tag,
                info,
                bool(info),
                prof.mHead->mNext.load(std::memory_order_relaxed)),
      std::memory_order_relaxed);
  }

  return prof;
}

Profiler::Entry&
Profiler::time(const char* tag, Info* info, bool owned) noexcept
{
  assert(info || !owned); // owned 无效时 info 必须为空

  auto* entry = new Entry(Clock::now(), tag, info, owned, nullptr);
  auto* next = mHead->mNext.load(std::memory_order_relaxed);
  do {
    entry->mNext.store(next, std::memory_order_relaxed);
  } while (!mHead->mNext.compare_exchange_weak(
    next, entry, std::memory_order_relaxed));
  return *entry;
}

bj::value
Profiler::to_json() const noexcept(false)
{
  bj::array arr;

  for (auto&& i : *this) {
    bj::array ent;
    ent.emplace_back(i.mTag);
    sc::duration<double, std::nano> dura(i.mTime - mHead->mTime);
    ent.emplace_back(dura.count());
    if (i.mInfo)
      ent.emplace_back(i.mInfo->info());
  }

  return arr;
}

Profiler::Scope::EnterInfo Profiler::Scope::gEnterInfo;
Profiler::Scope::LeaveInfo Profiler::Scope::gLeaveInfo;

Profiler::Entry::~Entry() noexcept
{
  if (mOwned)
    delete mInfo;

  // 无需原子操作，因为析构操作只发生在最后持有 shared_ptr 的单个线程中。
  auto* next = mNext.load(std::memory_order_relaxed);
  if (next) {
    while (next != nullptr) {
      auto* nextnext = next->mNext.load(std::memory_order_relaxed);
      next->mNext.store(nullptr, std::memory_order_relaxed);
      delete next; // 先将 next 置空，防止陷入递归析构。
      next = nextnext;
    }
  }
}

std::string
Profiler::Scope::EnterInfo::info() noexcept
{
  return "ENTER";
}

std::string
Profiler::Scope::LeaveInfo::info() noexcept
{
  return "LEAVE";
}

} // namespace Lib
