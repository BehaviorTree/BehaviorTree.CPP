#pragma once

#include <memory>
#include <mutex>

namespace BT
{
/**
 * @brief The LockedPtr class is used to share a pointer to an object
 * and a mutex that protects the read/write access to that object.
 *
 * As long as the object remains in scope, the mutex is locked, therefore
 * you must destroy this object as soon as the pointer was used.
 */
template <typename T>
class LockedPtr {
  public:

  LockedPtr() = default;

  LockedPtr(const T* obj, std::mutex* obj_mutex):
        ref_(obj), mutex_(obj_mutex) {
    mutex_->lock();
  }

  ~LockedPtr() {
    if(mutex_) {
      mutex_->unlock();
    }
  }

  LockedPtr(LockedPtr const&) = delete;
  LockedPtr& operator=(LockedPtr const&) = delete;

  LockedPtr(LockedPtr && other) {
    std::swap(ref_, other.ref_);
    std::swap(mutex_, other.mutex_);
  }

  LockedPtr& operator=(LockedPtr&& other) {
    std::swap(ref_, other.ref_);
    std::swap(mutex_, other.mutex_);
  }

  operator bool() const {
    return ref_ != nullptr;
  }

  void lock() {
    if(mutex_) {
      mutex_->lock();
    }
  }

  void unlock() {
    if(mutex_) {
      mutex_->unlock();
    }
  }

  bool empty() const {
    return ref_ == nullptr;
  }

  const T* get() const{
    return ref_;
  }

  private:
  const T* ref_ = nullptr;
  std::mutex* mutex_ = nullptr;
};


}
