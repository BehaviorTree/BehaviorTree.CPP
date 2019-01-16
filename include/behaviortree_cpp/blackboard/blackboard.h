#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>

#include "behaviortree_cpp/blackboard/safe_any.hpp"


namespace BT
{
// This is the "backend" of the blackboard.
// To create a new blackboard, user must inherit from BlackboardImpl
// and override set and get.
class BlackboardImpl
{
  public:
    virtual ~BlackboardImpl() = default;

    virtual const SafeAny::Any* get(const std::string& key) const = 0;
    virtual void set(const std::string& key, const SafeAny::Any& value) = 0;
    virtual bool contains(const std::string& key) const = 0;
};

// This is the "frontend" to be used by the developer.
//
// Even if the abstract class BlackboardImpl can be used directly,
// the templatized methods set() and get() are more user-friendly
class Blackboard
{
    // This is intentionally private. Use Blackboard::create instead
    Blackboard(std::unique_ptr<BlackboardImpl> base) : impl_(std::move(base))
    {
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
     *
     * return true if succesful
     */
    template <typename T>
    bool get(const std::string& key, T& value) const
    {
        if (!impl_)
        {
            return false;
        }
        const SafeAny::Any* val = impl_->get(key);
        if (!val)
        {
            return false;
        }

        value = val->cast<T>();
        return true;
    }

    const SafeAny::Any* getAny(const std::string& key) const
    {
        if (!impl_)
        {
            return nullptr;
        }
        return impl_->get(key);
    }


    template <typename T>
    T get(const std::string& key) const
    {
        T value;
        bool found = get(key, value);
        if (!found)
        {
            throw std::runtime_error("Missing key");
        }
        return value;
    }

    /// Update the entry with the given key
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        if (impl_)
        {
            impl_->set(key, SafeAny::Any(value));
        }
        else{
            throw  std::runtime_error("called set on an invalid BlackBoard");
        }
    }

    bool contains(const std::string& key) const
    {
        return (impl_ && impl_->contains(key));
    }

  private:
    std::unique_ptr<BlackboardImpl> impl_;
};
}

#endif   // BLACKBOARD_H
