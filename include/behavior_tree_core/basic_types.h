#ifndef BT_BASIC_TYPES_H
#define BT_BASIC_TYPES_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>

namespace BT
{
// Enumerates the possible types of a node, for drawinf we
// have do discriminate whoich control node it is:
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
// time step:
// - "Success" indicates that the node has completed running during this time step;
// - "Failure" indicates that the node has determined it will not be able to complete
//   its task;
// - "Running" indicates that the node has successfully moved forward during this
//   time step, but the task is not yet complete;
// - "Idle" indicates that the node hasn't run yet.
// - "Halted" indicates that the node has been halted by its father.
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
enum ResetPolicy
{
    ON_SUCCESS_OR_FAILURE,
    ON_SUCCESS,
    ON_FAILURE
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


template <typename T> T convertFromString(const std::string& str);

//------------------------------------------------------------------

/**
 * @brief toStr converts NodeStatus to string. Optionally colored.
 */
const char* toStr(const BT::NodeStatus& status, bool colored = false);

inline std::ostream& operator<<(std::ostream& os, const BT::NodeStatus& status)
{
    os << toStr(status);
    return os;
}

/**
 * @brief toStr converts NodeType to string.
 */
const char* toStr(const BT::NodeType& type);

inline std::ostream& operator<<(std::ostream& os, const BT::NodeType& type)
{
    os << toStr(type);
    return os;
}

/**
 * @brief toStr converts ResetPolicy to string.
 */
const char* toStr(const BT::ResetPolicy& policy);

inline std::ostream& operator<<(std::ostream& os, const BT::ResetPolicy& type)
{
    os << toStr(type);
    return os;
}

// small utility, unless you want to use <boost/algorithm/string.hpp>
inline std::vector<std::string> splitString(const std::string& strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splitted_strings;
    while (std::getline(ss, item, delimeter))
    {
       splitted_strings.push_back(item);
    }
    return splitted_strings;
}


}

#endif   // BT_BASIC_TYPES_H
