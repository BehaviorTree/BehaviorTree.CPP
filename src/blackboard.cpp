#include "behaviortree_cpp/blackboard.h"

namespace BT{

void Blackboard::setPortType(std::string key, const std::type_info *new_type)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        storage_.insert( { key, Entry(new_type)} );
    }
    else{
        auto old_type = it->second.locked_port_type;
        if( old_type && old_type != new_type )
        {
            throw LogicError( "Blackboard::set() failed: once declared, the type of a port shall not change. "
                             "Declared type [",     BT::demangle( old_type->name() ),
                             "] != current type [", BT::demangle( new_type->name() ), "]" );
        }
    }
}

const std::type_info *Blackboard::portType(const std::string &key)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        return nullptr;
    }
    return it->second.locked_port_type;
}

void Blackboard::addSubtreeRemapping(std::string internal, std::string external)
{
    internal_to_external_.insert( {std::move(internal), std::move(external)} );
}

void Blackboard::debugMessage() const
{
    std::cout << " -------------- " << std::endl;
    for(const auto& entry_it: storage_)
    {
        std::cout << entry_it.first << " / " <<demangle( entry_it.second.value.type().name() )
                  << " / " << entry_it.second.value.empty() <<  std::endl;
    }
    std::cout << " -------------- " << std::endl;
}

}
