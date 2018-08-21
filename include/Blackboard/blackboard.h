#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>

#include <SafeAny/safe_any.hpp>
#include <non_std/optional.hpp>

namespace BT{

class BlackboardImpl
{
public:

    virtual ~BlackboardImpl() = default;

    virtual const SafeAny::Any* get(const std::string& key) const = 0;
    virtual void set(const std::string& key, const SafeAny::Any& value) = 0;
};


// Blackboard is the front-end to be used by the developer.
// Even if the abstract class BlackboardImpl can be used directly,
// the templatized methods set() and get() are more user-friendly
class Blackboard
{


public:
    // This is intentionally private. Use Blackboard::create instead
    Blackboard(std::unique_ptr<BlackboardImpl> base ): impl_( std::move(base) )
    { }

    Blackboard() = delete;

    template <typename ImplClass, typename ... Args>
    static std::shared_ptr<Blackboard> create(Args... args )
    {
        std::unique_ptr<BlackboardImpl> base( new ImplClass(args...) );
        return std::make_shared<Blackboard>(std::move(base));
    }

    virtual ~Blackboard() = default;

    template <typename T> bool get(const std::string& key, T& value) const
    {
        if( !impl_ )
        {
            return false;
        }
        const SafeAny::Any* val = impl_->get(key);
        if( !val ){ return false; }

        value = val->cast<T>();
        return true;
    }

    template <typename T> void set(const std::string& key, const T& value) {
        if( impl_)
        {
            impl_->set(key, SafeAny::Any(value));
        }
    }

private:

    std::unique_ptr<BlackboardImpl> impl_;
};

}


#endif // BLACKBOARD_H
