#include <Exceptions.h>

using namespace BT;

BehaviorTreeException::BehaviorTreeException(const std::string Message)
{
    this->Message = std::string("BehaviorTreeException: " + Message).c_str();
}

const char* BehaviorTreeException::what()
{
    return Message;
}
