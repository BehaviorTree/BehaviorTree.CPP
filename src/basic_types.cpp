#include "behaviortree_cpp/basic_types.h"
#include <cstdlib>
#include <cstring>

namespace BT
{
const char* toStr(const NodeStatus& status, bool colored)
{
    if (!colored)
    {
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
    else
    {
        switch (status)
        {
            case NodeStatus::SUCCESS:
                return ("\x1b[32m"
                        "SUCCESS"
                        "\x1b[0m");   // RED
            case NodeStatus::FAILURE:
                return ("\x1b[31m"
                        "FAILURE"
                        "\x1b[0m");   // GREEN
            case NodeStatus::RUNNING:
                return ("\x1b[33m"
                        "RUNNING"
                        "\x1b[0m");   // YELLOW
            case NodeStatus::IDLE:
                return ("\x1b[36m"
                        "IDLE"
                        "\x1b[0m");   // CYAN
        }
    }
    return "Undefined";
}

const char* toStr(const NodeType& type)
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

template <>
std::string convertFromString<std::string>(StringView str)
{
    return std::string( str.data(), str.size() );
}

template <>
const char* convertFromString<const char*>(StringView str)
{
    return str.to_string().c_str();
}

template <>
int convertFromString<int>(StringView str)
{
    return  std::stoi(str.data());
}

template <>
unsigned convertFromString<unsigned>(StringView str)
{
    return std::stoul(str.data());
}

template <>
double convertFromString<double>(StringView str)
{
    return std::stod(str.data());
}

template <>
std::vector<int> convertFromString<std::vector<int>>(StringView str)
{
    auto parts = splitString(str, ';');
    std::vector<int> output;
    output.reserve( parts.size() );
    for(const StringView& part: parts)
    {
        char* end;
        output.push_back( std::strtol( part.data(), &end, 10 ) );
    }
    return output;
}

template <>
std::vector<double> convertFromString<std::vector<double>>(StringView str)
{
    auto parts = splitString(str, ';');
    std::vector<double> output;
    output.reserve( parts.size() );
    for(const StringView& part: parts)
    {
        char* end;
        output.push_back( std::strtod( part.data(), &end ) );
    }
    return output;
}

template <>
bool convertFromString<bool>(StringView str)
{
    if (str.size() == 1)
    {
        if (str[0] == '0')
        {
            return false;
        }
        else if (str[0] == '1')
        {
            return true;
        }
        else
        {
            std::runtime_error("invalid bool conversion");
        }
    }
    else if (str.size() == 4)
    {
        if (str == "true" || str == "TRUE" || str == "True")
        {
            return true;
        }
        else
        {
            std::runtime_error("invalid bool conversion");
        }
    }
    else if (str.size() == 5)
    {
        if (str == "false" || str == "FALSE" || str == "False")
        {
            return false;
        }
        else
        {
            std::runtime_error("invalid bool conversion");
        }
    }

    std::runtime_error("invalid bool conversion");
    return false;
}

template <>
NodeStatus convertFromString<NodeStatus>(StringView str)
{
    for (auto status :
         {NodeStatus::IDLE, NodeStatus::RUNNING, NodeStatus::SUCCESS, NodeStatus::FAILURE})
    {
        if ( StringView(toStr(status)) == str )
        {
            return status;
        }
    }
    throw std::invalid_argument(std::string("Cannot convert this to NodeStatus: ") + str.to_string() );
}

template <>
NodeType convertFromString<NodeType>(StringView str)
{
    for (auto status : {NodeType::ACTION, NodeType::CONDITION, NodeType::CONTROL,
                        NodeType::DECORATOR, NodeType::SUBTREE, NodeType::UNDEFINED})
    {
        if (StringView(toStr(status)) == str)
        {
            return status;
        }
    }
    throw std::invalid_argument(std::string("Cannot convert this to NodeType: ") + str.to_string());
}

std::ostream& operator<<(std::ostream& os, const NodeType& type)
{
    os << toStr(type);
    return os;
}

std::ostream& operator<<(std::ostream& os, const NodeStatus& status)
{
    os << toStr(status);
    return os;
}

std::vector<StringView> splitString(const StringView &strToSplit, char delimeter)
{
    std::vector<StringView> splitted_strings;
    splitted_strings.reserve(4);

    size_t pos = 0;
    while( pos < strToSplit.size())
    {
        size_t new_pos = strToSplit.find_first_of(delimeter, pos);
        if( new_pos == std::string::npos)
        {
           new_pos = strToSplit.size();
        }
        StringView sv = { &strToSplit.data()[pos], new_pos - pos };
        splitted_strings.push_back( sv );
        pos = new_pos + 1;
    }
    return splitted_strings;
}

}   // end namespace
