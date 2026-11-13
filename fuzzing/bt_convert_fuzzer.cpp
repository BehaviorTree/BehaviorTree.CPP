// Fuzz target: bt_convert_fuzzer
//
// Threat model: every XML attribute, script literal and blackboard value that
// enters a BehaviorTree.CPP application passes through BT::convertFromString<T>
// (port parsing). These string->type conversions are the numeric- and
// string-parsing funnel for untrusted data (attacker-supplied XML documents,
// Groot files, scripting expressions).
//
// Strategy: convertFromString<T> over the full builtin type matrix with a
// fuzzed string (numerics incl. int8..64/uint8..64/float/double overflow and
// underflow, bool, enums NodeStatus/NodeType/PortDirection, split containers
// vector<T>, SharedQueue<T>).
//
// All thrown BT exceptions (RuntimeError/LogicError on invalid input) are
// expected and caught; the point is exercising the conversion code paths, not
// crashing on them.
//
// Expected bug classes: integer/float overflow and truncation handling,
// out-of-bounds reads in split-string parsing, enum parsing mismatches.

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/decorators/loop_node.h"

#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 8)
  {
    return 0;
  }
  FuzzedDataProvider fdp(data, size);

  const std::string text = fdp.ConsumeRandomLengthString();
  const BT::StringView sv(text);

  try
  {
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 19))
    {
      case 0:
        (void)BT::convertFromString<std::string>(sv);
        break;
      case 1:
        (void)BT::convertFromString<int8_t>(sv);
        break;
      case 2:
        (void)BT::convertFromString<int16_t>(sv);
        break;
      case 3:
        (void)BT::convertFromString<int32_t>(sv);
        break;
      case 4:
        (void)BT::convertFromString<int64_t>(sv);
        break;
      case 5:
        (void)BT::convertFromString<uint8_t>(sv);
        break;
      case 6:
        (void)BT::convertFromString<uint16_t>(sv);
        break;
      case 7:
        (void)BT::convertFromString<uint32_t>(sv);
        break;
      case 8:
        (void)BT::convertFromString<uint64_t>(sv);
        break;
      case 9:
        (void)BT::convertFromString<float>(sv);
        break;
      case 10:
        (void)BT::convertFromString<double>(sv);
        break;
      case 11:
        (void)BT::convertFromString<bool>(sv);
        break;
      case 12:
        (void)BT::convertFromString<BT::NodeStatus>(sv);
        break;
      case 13:
        (void)BT::convertFromString<BT::NodeType>(sv);
        break;
      case 14:
        (void)BT::convertFromString<BT::PortDirection>(sv);
        break;
      case 15:
        (void)BT::convertFromString<std::vector<int>>(sv);
        break;
      case 16:
        (void)BT::convertFromString<std::vector<double>>(sv);
        break;
      case 17:
        (void)BT::convertFromString<std::vector<bool>>(sv);
        break;
      case 18:
        (void)BT::convertFromString<std::vector<std::string>>(sv);
        break;
      default:
        (void)BT::convertFromString<BT::SharedQueue<int>>(sv);
        break;
    }
  }
  catch(const std::exception&)
  {
    // invalid numeric/enum input raises RuntimeError: expected
  }

  return 0;
}
