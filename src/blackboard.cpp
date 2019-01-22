#include "behaviortree_cpp/blackboard.h"

namespace BT{

void Blackboard::setPortType(std::string key, const std::type_info *new_type)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            parent->setPortType( remapping_it->second, new_type );
        }
    }

    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        storage_.insert( { std::move(key), Entry(new_type)} );
    }
    else{
        auto old_type = it->second.locked_port_type;
        if( old_type && old_type != new_type )
        {
            throw LogicError( "Blackboard::set() failed: once declared, the type of a port shall not change. "
                             "Declared type [",     BT::demangle( old_type ),
                             "] != current type [", BT::demangle( new_type ), "]" );
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
    for(const auto& entry_it: storage_)
    {
        auto port_type = entry_it.second.locked_port_type;
        if( !port_type )
        {
            port_type = &( entry_it.second.value.type() );
        }

        std::cout <<  entry_it.first << " (" << demangle( port_type ) << ") -> ";

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find( entry_it.first );
            if( remapping_it != internal_to_external_.end())
            {
                std::cout << "remapped to parent [" << remapping_it->second << "]" <<std::endl;
                continue;
            }
        }
        std::cout << ((entry_it.second.value.empty()) ? "empty" : "full") <<  std::endl;
    }
}

}
