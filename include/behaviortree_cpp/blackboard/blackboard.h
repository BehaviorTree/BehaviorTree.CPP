#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp/blackboard/safe_any.hpp"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{
// This is the "backend" of the blackboard.
// To create a new blackboard, user must inherit from BlackboardImpl
// and override set and get.
/**
 * @brief The BlackboardImpl is the "backend" of the blackboard.
 * To create a new blackboard, user must inherit from BlackboardImpl
 * and override set and get.
 */
class BlackboardImpl
{
  public:
    virtual ~BlackboardImpl() = default;

    virtual SafeAny::Any* get(const std::string& key) = 0;
    virtual const SafeAny::Any* get(const std::string& key) const = 0;
    virtual void set(const std::string& key, const SafeAny::Any& value) = 0;
    virtual bool contains(const std::string& key) const = 0;

    virtual BlackboardImpl* createOther()  const = 0;
};


/**
 * @brief The Blackboard is the mechanism used by BehaviorTrees to exchange
 * typed data.
 *
 * This is the "frontend" to be used by the developer. The actual implementation
 * is defined as a derived class of BlackboardImpl.
 */
class Blackboard
{
    // This is intentionally private. Use Blackboard::create instead
    Blackboard(std::unique_ptr<BlackboardImpl> base) : impl_(std::move(base))
    {
        if (!impl_)
        {
            throw LogicError("An empty BlackboardImpl passed to Blackboard");
        }
    }

  public:
    typedef std::shared_ptr<Blackboard> Ptr;

    Blackboard() = delete;

    /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
    template <typename ImplClass, typename... Args>
    static Blackboard::Ptr create(Args... args)
    {
        std::unique_ptr<BlackboardImpl> base(new ImplClass(args...));
        return std::shared_ptr<Blackboard>(new Blackboard(std::move(base)));
    }

    virtual ~Blackboard() = default;

    /** Return true if the entry with the given key was found.
     *  Note that this method may throw an exception if the cast to T failed.
     */
    template <typename T>
    bool get(const std::string& key, T& value) const
    {
        const SafeAny::Any* val = getAny(key);
        if (val)
        {
            value = val->cast<T>();
        }
        return (bool)val;
    }

    /**
     * @brief The method getAny allow the user to access directly the type
     * erased value.
     *
     * @return the pointer or nullptr if it fails.
     */
    SafeAny::Any* getAny(const std::string& key) const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto val = impl_->get(key);
        return val;
    }

    /**
     * Version of get() that throws if it fails.
     */
    template <typename T>
    T get(const std::string& key) const
    {
        T value;
        bool found = get(key, value);
        if (!found)
        {
            throw RuntimeError("Missing key");
        }
        return value;
    }

    /// Update the entry with the given key
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        SafeAny::Any* existin_entry = impl_->get(key);
        if( existin_entry )
        {
            const auto& prev_type =  existin_entry->type();
            SafeAny::Any any_val(value);
            const auto& new_type = any_val.type();
            if( prev_type != new_type )
            {
                std::stringstream ss;
                ss << "Blackboard::set() failed: once created, the type of a port must not change.";
                ss << " Previous: [ " <<  existin_entry->type().name() << " ] ";
                ss << " Current: [ " <<  typeid (T).name() << " ]";
                throw LogicError( ss.str() );
            }

            impl_->set(key, std::move(any_val));
        }
        impl_->set(key, SafeAny::Any(value));
    }

    /// Return true if the BB contains an entry with the given key.
    bool contains(const std::string& key) const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return (impl_->contains(key));
    }

    Blackboard::Ptr createOther() const
    {
        auto base = std::unique_ptr<BlackboardImpl>( impl_->createOther() );
        return std::shared_ptr<Blackboard>(new Blackboard(std::move(base)));
    }

  private:
    std::unique_ptr<BlackboardImpl> impl_;
    mutable std::mutex mutex_;
};

template <> inline
void Blackboard::set<SafeAny::Any>(const std::string& key, const SafeAny::Any& value)
{
    std::unique_lock<std::mutex> lock(mutex_);

    auto existin_entry = impl_->get(key);
    if( existin_entry )
    {
        if( existin_entry->type() != value.type() )
        {
            std::stringstream ss;
            ss << "Blackboard::set() failed: once created, the type of a port must not change.";
            ss << " Previous: [ " <<  existin_entry->type().name() << " ] ";
            ss << " Current: [ " <<   value.type().name() << " ]";
            throw LogicError( ss.str() );
        }
    }
    impl_->set(key, value);
}

} // end namespace

#endif   // BLACKBOARD_H
