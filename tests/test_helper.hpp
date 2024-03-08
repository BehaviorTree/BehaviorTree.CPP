#ifndef TEST_HELPER_HPP
#define TEST_HELPER_HPP

#include <cstdio>

#include "behaviortree_cpp/bt_factory.h"

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
    snprintf(str, sizeof str, "%s%c", name_prefix.c_str(), char('A' + i));
    int* counter_ptr = &(tick_counters[i]);
    factory.registerSimpleAction(str, std::bind(&TestTick, counter_ptr));
  }
}

#endif  // TEST_HELPER_HPP
