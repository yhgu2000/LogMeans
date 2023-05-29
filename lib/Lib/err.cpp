#include "err.hpp"

namespace Lib {

using namespace std::string_literals;

namespace err {

std::string
Errno::info() const noexcept
{
#ifdef _MSC_VER
  std::string msg(64, '\0');
  strerror_s(msg.data(), msg.size(), _code);
  msg.resize(msg.find('\0'));
  return msg;
#else
  std::string msg(64, '\0');
  strerror_r(_code, msg.data(), msg.size());
  msg.resize(msg.find('\0'));
  return msg;
#endif
}

}
}
