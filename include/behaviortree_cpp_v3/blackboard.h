#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp_v3/basic_types.h"
#include "behaviortree_cpp_v3/utils/safe_any.hpp"
#include "behaviortree_cpp_v3/exceptions.h"

namespace BT
{

/**
 * @brief The Blackboard is the mechanism used by BehaviorTrees to exchange
 * typed data.
 */
class Blackboard
{
  public:
    typedef std::shared_ptr<Blackboard> Ptr;

  protected:
    // This is intentionally protected. Use Blackboard::create instead
    Blackboard(Blackboard::Ptr parent): parent_bb_(parent)
    {}

  public:

    /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
    static Blackboard::Ptr create(Blackboard::Ptr parent = {})
    {
        return std::shared_ptr<Blackboard>( new Blackboard(parent) );
    }

    virtual ~Blackboard() = default;

    /**
     * @brief The method getAny allow the user to access directly the type
     * erased value.
     *
     * @return the pointer or nullptr if it fails.
     */
    const Any* getAny(const std::string& key) const
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                return parent->getAny( remapping_it->second );
            }
        }
        auto it = storage_.find(key);
        return ( it == storage_.end()) ? nullptr : &(it->second.value);
    }

    Any* getAny(const std::string& key)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                return parent->getAny( remapping_it->second );
            }
        }
        auto it = storage_.find(key);
        return ( it == storage_.end()) ? nullptr : &(it->second.value);
    }

    /** Return true if the entry with the given key was found.
     *  Note that this method may throw an exception if the cast to T failed.
     */
    template <typename T>
    bool get(const std::string& key, T& value) const
    {
        const Any* val = getAny(key);
        if (val)
        {
            value = val->cast<T>();
        }
        return (bool)val;
    }

    /**
     * Version of get() that throws if it fails.
    */
    template <typename T>
    T get(const std::string& key) const
    {
        const Any* val = getAny(key);
        if (val)
        {
            return val->cast<T>();
        }
        else
        {
            throw RuntimeError("Blackboard::get() error. Missing key [", key, "]");
        }
    }


    /// Update the entry with the given key
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = storage_.find(key);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                const auto& remapped_key = remapping_it->second;
                if( it == storage_.end() ) // virgin entry
                {
                    auto parent_info = parent->portInfo(remapped_key);
                    if( parent_info )
                    {
                        storage_.insert( {key, Entry( *parent_info ) } );
                    }
                    else{
                        storage_.insert( {key, Entry( PortInfo() ) } );
                    }
                }
                parent->set( remapped_key, value );
                return;
            }
        }

        if( it != storage_.end() ) // already there. check the type
        {
            const PortInfo& port_info = it->second.port_info;
            auto& previous_any = it->second.value;
            const auto locked_type = port_info.type();

            Any temp(value);

            if( locked_type && locked_type != &typeid(T) && locked_type != &temp.type() )
            {
                bool mismatching = true;
                if( std::is_constructible<StringView, T>::value )
                {
                    Any any_from_string = port_info.parseString( value );
                    if( any_from_string.empty() == false)
                    {
                        mismatching = false;
                        temp = std::move( any_from_string );
                    }
                }

                if( mismatching )
                {
                    debugMessage();

                    throw LogicError( "Blackboard::set() failed: once declared, the type of a port shall not change. "
                                     "Declared type [", demangle( locked_type ),
                                     "] != current type [", demangle( typeid(T) ),"]" );
                }
            }
            previous_any = std::move(temp);
        }
        else{ // create for the first time without any info
            storage_.emplace( key, Entry( Any(value), PortInfo() ) );
        }
        return;
    }

    void setPortInfo(std::string key, const PortInfo& info);

    const PortInfo *portInfo(const std::string& key);

    void addSubtreeRemapping(std::string internal, std::string external);

    void debugMessage() const;

  private:

    struct Entry{
        Any value;
        const PortInfo port_info;

        Entry( const PortInfo& info ):
          port_info(info)
        {}

        Entry(Any&& other_any, const PortInfo& info):
          value(std::move(other_any)),
          port_info(info)
        {}
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Entry> storage_;
    std::weak_ptr<Blackboard> parent_bb_;
    std::unordered_map<std::string,std::string> internal_to_external_;

};


} // end namespace

#endif   // BLACKBOARD_H
