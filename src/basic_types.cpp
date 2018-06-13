#include "behavior_tree_core/basic_types.h"
#include <cstring>

namespace BT{

const char *toStr(const NodeStatus &status, bool colored)
{
    if( ! colored ){
        switch (status)
        {
        case NodeStatus::SUCCESS:
            return "SUCCESS";
        case NodeStatus::FAILURE:
            return "FAILURE";
        case NodeStatus::RUNNING:
            return "RUNNING";
        case NodeStatus::IDLE:
            return "IDLE";
        }
    }
    else{
        switch (status)
        {
        case NodeStatus::SUCCESS:
            return ( "\x1b[32m" "SUCCESS" "\x1b[0m"); // RED
        case NodeStatus::FAILURE:
            return ( "\x1b[31m" "FAILURE" "\x1b[0m"); // GREEN
        case NodeStatus::RUNNING:
            return ( "\x1b[33m" "RUNNING" "\x1b[0m"); // YELLOW
        case NodeStatus::IDLE:
            return  ( "\x1b[36m" "IDLE" "\x1b[0m"); // CYAN
        }
    }
    return "Undefined";
}

const char *toStr(const NodeType &type)
{
    switch (type)
    {
    case NodeType::ACTION:
        return "Action";
    case NodeType::CONDITION:
        return "Condition";
    case NodeType::DECORATOR:
        return "Decorator";
    case NodeType::CONTROL:
        return "Control";
    case NodeType::SUBTREE:
        return "SubTree";
    default:
        return "Undefined";
    }
}

const char *toStr(const ResetPolicy &policy)
{
    switch (policy)
    {
    case ResetPolicy::ON_FAILURE:
        return "ON_FAILURE";
    case ResetPolicy::ON_SUCCESS:
        return "ON_SUCCESS";
    case ResetPolicy::ON_SUCCESS_OR_FAILURE:
        return "ON_SUCCESS_OR_FAILURE";
    default:
        return "Undefined";
    }
}

template<> int convertFromString<int>(const char* str)
{
    return std::stoi(str);
}

template<> unsigned convertFromString<unsigned>(const char* str)
{
    return std::stoul(str);
}

template<> double convertFromString<double>(const char* str)
{
    return std::stod(str);
}

template<> NodeStatus convertFromString<NodeStatus>(const char* str)
{
    for( auto status: {
         NodeStatus::IDLE,
         NodeStatus::RUNNING,
         NodeStatus::SUCCESS,
         NodeStatus::FAILURE} )
    {
        if( std::strcmp( toStr(status), str) == 0 )
        {
            return status;
        }
    }
    throw std::invalid_argument( std::string("Cannot convert this to NodeStatus: ") + str );
}

template<> NodeType convertFromString<NodeType>(const char* str)
{
    for( auto status: {
         NodeType::ACTION,
         NodeType::CONDITION,
         NodeType::CONTROL,
         NodeType::DECORATOR,
         NodeType::SUBTREE,
         NodeType::UNDEFINED} )
    {
        if( std::strcmp( toStr(status), str) == 0 )
        {
            return status;
        }
    }
    throw std::invalid_argument( std::string("Cannot convert this to NodeType: ") + str );
}

template<> ResetPolicy convertFromString<ResetPolicy>(const char* str)
{
    for( auto status: {
         ResetPolicy::ON_SUCCESS,
         ResetPolicy::ON_SUCCESS_OR_FAILURE,
         ResetPolicy::ON_FAILURE} )
    {
        if( std::strcmp( toStr(status), str) == 0 )
        {
            return status;
        }
    }
    throw std::invalid_argument( std::string("Cannot convert this to ResetPolicy: ") + str );
}

} // end namespace

