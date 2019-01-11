#ifndef BT_BASIC_TYPES_H
#define BT_BASIC_TYPES_H

#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <functional>
#include <chrono>
#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/string_view.hpp"
#include "behaviortree_cpp/blackboard/safe_any.hpp"

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
    RUNNING,
    SUCCESS,
    FAILURE
};

typedef nonstd::string_view StringView;

/**
 * convertFromString is used to convert a string into a custom type.
 *
 * This function is invoked under the hood by TreeNode::getInput(), but only when the
 * input port contains a string.
 *
 * If you have a custom type, you need to implement the corresponding template specialization.
 */
template <typename T> inline
T convertFromString(StringView /*str*/)
{
    auto type_name = BT::demangle( typeid(T).name() );

    std::cerr << "You (maybe indirectly) called BT::convertFromString() for type [" <<
                 type_name <<"], but I can't find the template specialization.\n" << std::endl;

    throw LogicError(std::string("You didn't implement the template specialization of "
                                 "convertFromString for this type: ") + type_name );
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
double convertFromString<double>(StringView str);

template <> // Integer numbers separated by the characted ";"
std::vector<int> convertFromString<std::vector<int>>(StringView str);

template <> // Real numbers separated by the characted ";"
std::vector<double> convertFromString<std::vector<double>>(StringView str);

template <> // This recognizes either 0/1, true/false, TRUE/FALSE
bool convertFromString<bool>(StringView str);

template <> // Names with all capital letters
NodeStatus convertFromString<NodeStatus>(StringView str);

template <>  // Names with all capital letters
NodeType convertFromString<NodeType>(StringView str);


//------------------------------------------------------------------

/**
 * @brief toStr converts NodeStatus to string. Optionally colored.
 */
const char* toStr(const BT::NodeStatus& status, bool colored = false);

std::ostream& operator<<(std::ostream& os, const BT::NodeStatus& status);

/**
 * @brief toStr converts NodeType to string.
 */
const char* toStr(const BT::NodeType& type);

std::ostream& operator<<(std::ostream& os, const BT::NodeType& type);


// Small utility, unless you want to use <boost/algorithm/string.hpp>
std::vector<StringView> splitString(const StringView& strToSplit, char delimeter);

template <typename Predicate>
using enable_if = typename std::enable_if< Predicate::value >::type*;

template <typename Predicate>
using enable_if_not = typename std::enable_if< !Predicate::value >::type*;

enum class PortType{INPUT, OUTPUT, INOUT };

class PortInfo
{
    template<typename T>
    PortInfo( PortType direction, T*):
        _type(direction),
        _info(typeid(T)),
        _empty_any(T())
    { }

public:
    PortInfo( PortType direction ): _type(direction), _info(typeid(void)) {}

    PortType type() const;

    const std::type_info &info() const;

    SafeAny::Any createEmptyAny() const
    {
        return _empty_any;
    }

    template<typename T> static PortInfo createInputPort()
    {
        return PortInfo(PortType::INPUT, static_cast<T*>(nullptr));
    }

    template<typename T> static PortInfo createOutputPort()
    {
        return PortInfo(PortType::OUTPUT, static_cast<T*>(nullptr));
    }

    template<typename T> static PortInfo createInoutPort()
    {
        return PortInfo(PortType::INOUT, static_cast<T*>(nullptr));
    }

private:

    PortType _type;
    const std::type_info& _info;
    SafeAny::Any _empty_any;
};


typedef std::unordered_map<std::string, PortInfo> PortsList;

template <typename T, typename = void>
struct has_static_method_providedPorts: std::false_type {};

template <typename T>
struct has_static_method_providedPorts<T,
        typename std::enable_if<std::is_same<decltype(T::providedPorts()), const PortsList&>::value>::type>
    : std::true_type {};

template <typename T> inline
PortsList getProvidedPorts(enable_if< has_static_method_providedPorts<T> > = nullptr)
{
    return T::providedPorts();
}

template <typename T> inline
PortsList getProvidedPorts(enable_if_not< has_static_method_providedPorts<T> > = nullptr)
{
    return {};
}

typedef std::chrono::high_resolution_clock::time_point TimePoint;
typedef std::chrono::high_resolution_clock::duration Duration;

} // end namespace


#endif   // BT_BASIC_TYPES_H
