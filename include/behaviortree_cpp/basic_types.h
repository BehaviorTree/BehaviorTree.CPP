#ifndef BT_BASIC_TYPES_H
#define BT_BASIC_TYPES_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <exception>
#include "behaviortree_cpp/string_view.hpp"
#include "behaviortree_cpp/blackboard/demangle_util.h"

namespace BT
{
// Enumerates the possible types of nodes
enum class NodeType
{
    UNDEFINED = 0,
    ACTION,
    CONDITION,
    CONTROL,
    DECORATOR,
    SUBTREE
};

// Enumerates the states every node can be in after execution during a particular
// time step.
enum class NodeStatus
{
    IDLE = 0,
    RUNNING,
    SUCCESS,
    FAILURE
};

// Enumerates the options for when a parallel node is considered to have failed:
// - "FAIL_ON_ONE" indicates that the node will return failure as soon as one of
//   its children fails;
// - "FAIL_ON_ALL" indicates that all of the node's children must fail before it
//   returns failure.
enum FailurePolicy
{
    FAIL_ON_ONE,
    FAIL_ON_ALL
};

// Enumerates the options for when a parallel node is considered to have succeeded:
// - "SUCCEED_ON_ONE" indicates that the node will return success as soon as one
//   of its children succeeds;
// - "BT::SUCCEED_ON_ALL" indicates that all of the node's children must succeed before
//   it returns success.
enum SuccessPolicy
{
    SUCCEED_ON_ONE,
    SUCCEED_ON_ALL
};

typedef nonstd::string_view StringView;

/// TreeNode::getParam requires convertFromString to be implemented for your specific type,
/// unless you are try to read it from a blackboard.
///
template <typename T> inline
T convertFromString(const StringView& /*str*/)
{
    auto type_name = BT::demangle( typeid(T).name() );

    std::cerr << "You (maybe indirectly) called BT::convertFromString() for type [" <<
                 type_name <<"], but I can't find the template specialization.\n" << std::endl;

    throw std::logic_error(std::string("You didn't implement the template specialization of "
                                       "convertFromString for this type: ") + type_name );
}

template <>
std::string convertFromString<std::string>(const StringView& str);

template <>
const char* convertFromString<const char*>(const StringView& str);

template <>
int convertFromString<int>(const StringView& str);

template <>
unsigned convertFromString<unsigned>(const StringView& str);

template <>
double convertFromString<double>(const StringView& str);

template <> // Integer numbers separated by the characted ";"
std::vector<int> convertFromString<std::vector<int>>(const StringView& str);

template <> // Real numbers separated by the characted ";"
std::vector<double> convertFromString<std::vector<double>>(const StringView& str);

template <> // This recognizes either 0/1, true/false, TRUE/FALSE
bool convertFromString<bool>(const StringView& str);

template <> // Names with all capital letters
NodeStatus convertFromString<NodeStatus>(const StringView& str);

template <>  // Names with all capital letters
NodeType convertFromString<NodeType>(const StringView& str);


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


// small utility, unless you want to use <boost/algorithm/string.hpp>
std::vector<StringView> splitString(const StringView& strToSplit, char delimeter);
}

#endif   // BT_BASIC_TYPES_H
