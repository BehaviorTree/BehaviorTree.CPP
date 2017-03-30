#include <exceptions.h>


BT::BehaviorTreeException::BehaviorTreeException(const std::string Message)
{
    this->Message = std::string("BehaviorTreeException: " + Message).c_str();
}

const char* BT::BehaviorTreeException::what()
{
    return Message;
}
