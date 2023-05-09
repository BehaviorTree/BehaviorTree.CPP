#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

namespace BT
{
template <typename T>
class LockedRef {
  public:

  LockedRef() = default;

  LockedRef(T* obj, std::shared_mutex* obj_mutex):
        ref_(obj), mutex_(obj_mutex) {
    mutex_->lock();
  }

  ~LockedRef() {
    if(mutex_) {
      mutex_->unlock();
    }
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
  T* get() {
    return ref_;
  }

  private:
  T* ref_ = nullptr;
  std::shared_mutex* mutex_ = nullptr;
};


template <typename T>
class LockedConstRef {
  public:

  LockedConstRef() = default;

  LockedConstRef(LockedConstRef const&) = delete;
  LockedConstRef& operator=(LockedConstRef const&) = delete;

  LockedConstRef(const T* obj, std::shared_mutex* obj_mutex):
        ref_(obj), mutex_(obj_mutex) {
    mutex_->lock_shared();
  }

  ~LockedConstRef() {
    if(mutex_) {
      mutex_->unlock_shared();
    }
  }

  operator bool() const {
    return ref_ != nullptr;
  }

  bool empty() const {
    return ref_ == nullptr;
  }

  const T* get() const {
    return ref_;
  }

  private:
  const T* ref_ = nullptr;
  std::shared_mutex* mutex_ = nullptr;
};



}
