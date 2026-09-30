#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include "behaviortree_cpp/bt_factory.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#if !defined(_WIN32)
#include <locale.h>
#if defined(__APPLE__)
#include <xlocale.h>
#endif
#endif

/**
 * Switch the calling thread (and only this thread) to a locale whose decimal
 * separator is ',' for the duration of the scope. Use active() to skip the test
 * when no such locale is available on the machine.
 */
class ScopedCommaDecimalLocale
{
public:
  ScopedCommaDecimalLocale()
  {
#if !defined(_WIN32)
    locale_ = ::newlocale(LC_NUMERIC_MASK, "de_DE.UTF-8", static_cast<::locale_t>(0));
    if(locale_ != static_cast<::locale_t>(0))
    {
      previous_ = ::uselocale(locale_);
    }
#endif
  }

  ~ScopedCommaDecimalLocale()
  {
#if !defined(_WIN32)
    if(locale_ != static_cast<::locale_t>(0))
    {
      ::uselocale(previous_);
      ::freelocale(locale_);
    }
#endif
  }

  ScopedCommaDecimalLocale(const ScopedCommaDecimalLocale&) = delete;
  ScopedCommaDecimalLocale& operator=(const ScopedCommaDecimalLocale&) = delete;

  /// true if the C library now parses "0,5" as one half on this thread
  [[nodiscard]] bool active() const
  {
#if !defined(_WIN32)
    return locale_ != static_cast<::locale_t>(0) && std::strtod("0,5", nullptr) == 0.5;
#else
    return false;
#endif
  }

private:
#if !defined(_WIN32)
  ::locale_t locale_ = static_cast<::locale_t>(0);
  ::locale_t previous_ = static_cast<::locale_t>(0);
#endif
};

inline BT::NodeStatus TestTick(int* tick_counter)
{
  (*tick_counter)++;
  return BT::NodeStatus::SUCCESS;
}

template <size_t N>
inline void RegisterTestTick(BT::BehaviorTreeFactory& factory,
                             const std::string& name_prefix,
                             std::array<int, N>& tick_counters)
{
  for(size_t i = 0; i < tick_counters.size(); i++)
  {
    tick_counters[i] = false;
    char str[100];
    (void)snprintf(str, sizeof str, "%s%c", name_prefix.c_str(), char('A' + i));
    int* counter_ptr = &(tick_counters[i]);
    factory.registerSimpleAction(str, std::bind(&TestTick, counter_ptr));
  }
}

#endif  // TEST_HELPER_HPP
