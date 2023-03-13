#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <functional>
#include <chrono>
#include <memory>
#include <string_view>
#include <variant>
#include <optional>

#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/utils/expected.hpp"

namespace BT
{
/// Enumerates the possible types of nodes
enum class NodeType
{
  UNDEFINED = 0,
  ACTION,
  CONDITION,
  CONTROL,
  DECORATOR,
  SUBTREE
};

/// Enumerates the states every node can be in after execution during a particular
/// time step.
/// IMPORTANT: Your custom nodes should NEVER return IDLE.
enum class NodeStatus
{
  IDLE = 0,
  RUNNING = 1,
  SUCCESS = 2,
  FAILURE = 3,
  SKIPPED = 4,
};

inline bool isStatusActive(const NodeStatus& status)
{
  return status != NodeStatus::IDLE && status != NodeStatus::SKIPPED;
}

inline bool isStatusCompleted(const NodeStatus& status)
{
  return status == NodeStatus::SUCCESS || status == NodeStatus::FAILURE;
}

enum class PortDirection
{
  INPUT,
  OUTPUT,
  INOUT
};

using StringView = std::string_view;

/**
 * convertFromString is used to convert a string into a custom type.
 *
 * This function is invoked under the hood by TreeNode::getInput(), but only when the
 * input port contains a string.
 *
 * If you have a custom type, you need to implement the corresponding template specialization.
 */
template <typename T> [[nodiscard]]
inline T convertFromString(StringView /*str*/)
{
  auto type_name = BT::demangle(typeid(T));

  std::cerr << "You (maybe indirectly) called BT::convertFromString() for type ["
            << type_name << "], but I can't find the template specialization.\n"
            << std::endl;

  throw LogicError(std::string("You didn't implement the template specialization of "
                               "convertFromString for this type: ") +
                   type_name);
}

template <>
std::string convertFromString<std::string>(StringView str);

template <>
const char* convertFromString<const char*>(StringView str);

template <>
int convertFromString<int>(StringView str);

template <>
unsigned convertFromString<unsigned>(StringView str);

template <>
long convertFromString<long>(StringView str);

template <>
unsigned long convertFromString<unsigned long>(StringView str);

template <>
float convertFromString<float>(StringView str);

template <>
double convertFromString<double>(StringView str);

template <>   // Integer numbers separated by the character ";"
std::vector<int> convertFromString<std::vector<int>>(StringView str);

template <>   // Real numbers separated by the character ";"
std::vector<double> convertFromString<std::vector<double>>(StringView str);

template <>   // This recognizes either 0/1, true/false, TRUE/FALSE
bool convertFromString<bool>(StringView str);

template <>   // Names with all capital letters
NodeStatus convertFromString<NodeStatus>(StringView str);

template <>   // Names with all capital letters
NodeType convertFromString<NodeType>(StringView str);

template <>
PortDirection convertFromString<PortDirection>(StringView str);

typedef std::function<Any(StringView)> StringConverter;

typedef std::unordered_map<const std::type_info*, StringConverter> StringConvertersMap;

// helper function
template <typename T>
inline StringConverter GetAnyFromStringFunctor()
{
  return [](StringView str) { return Any(convertFromString<T>(str)); };
}

template <>
inline StringConverter GetAnyFromStringFunctor<void>()
{
  return {};
}

//------------------------------------------------------------------

template <typename T> [[nodiscard]]
std::string toStr(T value)
{
  return std::to_string(value);
}

std::string toStr(const std::string& value);

template <>
std::string toStr<BT::NodeStatus>(BT::NodeStatus status);

/**
 * @brief toStr converts NodeStatus to string. Optionally colored.
 */
std::string toStr(BT::NodeStatus status, bool colored);

std::ostream& operator<<(std::ostream& os, const BT::NodeStatus& status);

/**
 * @brief toStr converts NodeType to string.
 */
template <> [[nodiscard]]
std::string toStr<BT::NodeType>(BT::NodeType type);

std::ostream& operator<<(std::ostream& os, const BT::NodeType& type);

template <> [[nodiscard]]
std::string toStr<BT::PortDirection>(BT::PortDirection direction);

std::ostream& operator<<(std::ostream& os, const BT::PortDirection& type);

// Small utility, unless you want to use <boost/algorithm/string.hpp>
[[nodiscard]]
std::vector<StringView> splitString(const StringView& strToSplit, char delimeter);

template <typename Predicate>
using enable_if = typename std::enable_if<Predicate::value>::type*;

template <typename Predicate>
using enable_if_not = typename std::enable_if<!Predicate::value>::type*;

/** Usage: given a function/method like this:
 *
 *     Expected<double> getAnswer();
 *
 * User code can check result and error message like this:
 *
 *     auto res = getAnswer();
 *     if( res )
 *     {
 *         std::cout << "answer was: " << res.value() << std::endl;
 *     }
 *     else{
 *         std::cerr << "failed to get the answer: " << res.error() << std::endl;
 *     }
 *
 * */
template <typename T>
using Expected = nonstd::expected<T, std::string>;
#ifdef USE_BTCPP3_OLD_NAMES
// note: we also use the name Optional instead of expected because it is more intuitive
// for users that are not up to date with "modern" C++
template <typename T>
using Optional = nonstd::expected<T, std::string>;
#endif

/** Usage: given a function/method like:
 *
 *     Result DoSomething();
 *
 * User code can check result and error message like this:
 *
 *     auto res = DoSomething();
 *     if( res )
 *     {
 *         std::cout << "DoSomething() done " << std::endl;
 *     }
 *     else{
 *         std::cerr << "DoSomething() failed with message: " << res.error() << std::endl;
 *     }
 *
 * */
using Result = Expected<std::monostate>;

[[nodiscard]]
bool IsAllowedPortName(StringView str);

class PortInfo
{
public:
  struct AnyTypeAllowed
  {
  };

  PortInfo(PortDirection direction = PortDirection::INOUT) :
    _type(direction), _type_info(typeid(AnyTypeAllowed))
  {}

  PortInfo(PortDirection direction, std::type_index type_info, StringConverter conv) :
    _type(direction), _type_info(type_info), _converter(conv)
  {}

  [[nodiscard]] PortDirection direction() const;

  [[nodiscard]] const std::type_index& type() const;

  [[nodiscard]] Any parseString(const char* str) const;

  [[nodiscard]] Any parseString(const std::string& str) const;

  template <typename T> [[nodiscard]]
  Any parseString(const T&) const
  {
    // avoid compilation errors
    return {};
  }

  void setDescription(StringView description);

  void setDefaultValue(StringView default_value_as_string);

  [[nodiscard]] const std::string& description() const;

  [[nodiscard]] std::optional<std::string> defaultValue() const;

  [[nodiscard]] bool isStronglyTyped() const
  {
    return _type_info != typeid(AnyTypeAllowed);
  }

  [[nodiscard]] const StringConverter& converter() const
  {
    return _converter;
  }

private:
  PortDirection _type;
  std::type_index _type_info;
  StringConverter _converter;
  std::string description_;
  std::optional<std::string> default_value_;
};

template <typename T = PortInfo::AnyTypeAllowed> [[nodiscard]]
std::pair<std::string, PortInfo> CreatePort(PortDirection direction,
                                            StringView name,
                                            StringView description = {})
{
  auto sname = static_cast<std::string>(name);
  if (!IsAllowedPortName(sname))
  {
    throw RuntimeError("The name of a port must not be `name` or `ID` "
                       "and must start with an alphabetic character. "
                       "Underscore is reserved.");
  }

  std::pair<std::string, PortInfo> out;

  if (std::is_same<T, void>::value)
  {
    out = {sname, PortInfo(direction)};
  }
  else
  {
    out = {sname, PortInfo(direction, typeid(T), GetAnyFromStringFunctor<T>())};
  }
  if (!description.empty())
  {
    out.second.setDescription(description);
  }
  return out;
}

//----------
template <typename T = void> [[nodiscard]]
inline std::pair<std::string, PortInfo> InputPort(StringView name,
                                                  StringView description = {})
{
  return CreatePort<T>(PortDirection::INPUT, name, description);
}

template <typename T = void> [[nodiscard]]
inline std::pair<std::string, PortInfo> OutputPort(StringView name,
                                                   StringView description = {})
{
  return CreatePort<T>(PortDirection::OUTPUT, name, description);
}

template <typename T = void> [[nodiscard]]
inline std::pair<std::string, PortInfo> BidirectionalPort(StringView name,
                                                          StringView description = {})
{
  return CreatePort<T>(PortDirection::INOUT, name, description);
}
//----------
template <typename T = void> [[nodiscard]]
inline std::pair<std::string, PortInfo> InputPort(StringView name, const T& default_value,
                                                  StringView description)
{
  auto out = CreatePort<T>(PortDirection::INPUT, name, description);
  out.second.setDefaultValue(BT::toStr(default_value));
  return out;
}

template <typename T = void> [[nodiscard]]
inline std::pair<std::string, PortInfo> BidirectionalPort(StringView name,
                                                          const T& default_value,
                                                          StringView description)
{
  auto out = CreatePort<T>(PortDirection::INOUT, name, description);
  out.second.setDefaultValue(BT::toStr(default_value));
  return out;
}
//----------

typedef std::unordered_map<std::string, PortInfo> PortsList;

template <typename T, typename = void>
struct has_static_method_providedPorts : std::false_type
{
};

template <typename T>
struct has_static_method_providedPorts<
    T, typename std::enable_if<
           std::is_same<decltype(T::providedPorts()), PortsList>::value>::type>
  : std::true_type
{
};

template <typename T> [[nodiscard]]
inline PortsList getProvidedPorts(enable_if<has_static_method_providedPorts<T>> = nullptr)
{
  return T::providedPorts();
}

template <typename T> [[nodiscard]]
inline PortsList
    getProvidedPorts(enable_if_not<has_static_method_providedPorts<T>> = nullptr)
{
  return {};
}

typedef std::chrono::high_resolution_clock::time_point TimePoint;
typedef std::chrono::high_resolution_clock::duration Duration;

}   // namespace BT

