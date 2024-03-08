#pragma once

#include <mutex>
#include "behaviortree_cpp/utils/safe_any.hpp"

namespace BT
{
/**
 * @brief The LockedPtr class is used to share a pointer to an object
 * and a mutex that protects the read/write access to that object.
 *
 * As long as the object remains in scope, the mutex is locked, therefore
 * you must destroy this instance as soon as the pointer was used.
 */
template <typename T>
class LockedPtr
{
public:
  LockedPtr() = default;

  LockedPtr(T* obj, std::mutex* obj_mutex) : ref_(obj), mutex_(obj_mutex)
  {
    mutex_->lock();
  }

  ~LockedPtr()
  {
    if(mutex_)
    {
      mutex_->unlock();
    }
  }

  LockedPtr(LockedPtr const&) = delete;
  LockedPtr& operator=(LockedPtr const&) = delete;

  LockedPtr(LockedPtr&& other)
  {
    std::swap(ref_, other.ref_);
    std::swap(mutex_, other.mutex_);
  }

  LockedPtr& operator=(LockedPtr&& other)
  {
    std::swap(ref_, other.ref_);
    std::swap(mutex_, other.mutex_);
  }

  operator bool() const
  {
    return ref_ != nullptr;
  }

  void lock()
  {
    if(mutex_)
    {
      mutex_->lock();
    }
  }

  void unlock()
  {
    if(mutex_)
    {
      mutex_->unlock();
    }
  }

  const T* get() const
  {
    return ref_;
  }

  const T* operator->() const
  {
    return ref_;
  }

  T* operator->()
  {
    return ref_;
  }

  template <typename OtherT>
  void assign(const OtherT& other)
  {
    if(ref_ == nullptr)
    {
      throw std::runtime_error("Empty LockedPtr reference");
    }
    else if constexpr(std::is_same_v<T, OtherT>)
    {
      *ref_ = other;
    }
    else if constexpr(std::is_same_v<BT::Any, OtherT>)
    {
      other->copyInto(*ref_);
    }
    else
    {
      *ref_ = T(other);
    }
  }

private:
  T* ref_ = nullptr;
  std::mutex* mutex_ = nullptr;
};

}  // namespace BT
