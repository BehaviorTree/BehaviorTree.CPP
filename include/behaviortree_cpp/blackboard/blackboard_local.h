#ifndef BLACKBOARD_LOCAL_H
#define BLACKBOARD_LOCAL_H

#include "blackboard.h"

namespace BT
{
class BlackboardLocal : public BlackboardImpl
{
  public:
    BlackboardLocal()
    {
    }

    virtual const SafeAny::Any* get(const std::string& key) const override
    {
        auto it = storage_.find(key);
        return (it == storage_.end()) ? nullptr : &(it->second);
    }

    virtual SafeAny::Any* get(const std::string& key) override
    {
        auto it = storage_.find(key);
        return (it == storage_.end()) ? nullptr : &(it->second);
    }

    virtual void set(const std::string& key, const SafeAny::Any& value) override
    {
        storage_[key] = value;
    }

    virtual bool contains(const std::string& key) const override
    {
        return storage_.find(key) != storage_.end();
    }

    virtual BlackboardImpl* createOther()  const override
    {
        return new BlackboardLocal();
    }

  private:
    std::unordered_map<std::string, SafeAny::Any> storage_;
};
}

#endif   // BLACKBOARD_LOCAL_H
