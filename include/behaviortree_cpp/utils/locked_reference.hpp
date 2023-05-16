#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

namespace BT
{
/**
 * @brief The LockedPtr class is used to share a pointer to an object
 * and a mutex that protects the write access to that object.
 *
 * As long as the object remains in scope, the mutex is locked
 */
template <typename T>
class LockedPtr {
  public:

  LockedPtr() = default;

      LockedPtr(T* obj, std::shared_mutex* obj_mutex):
        ref_(obj), mutex_(obj_mutex) {
    mutex_->lock();
  }

      ~LockedPtr() {
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

/**
 * @brief The LockedPtrConst class is used to share a pointer to an object
 * and a mutex that protects the access to that object.
 * It has a read-only interface, the object can not be modified.
 * Multiple instances of LockedPtrConst can exist without a dead-lock.
 *
 * As long as the object remains in scope, the shared mutex is locked
 */
template <typename T>
class LockedPtrConst {
  public:

  LockedPtrConst() = default;

      LockedPtrConst(LockedPtrConst const&) = delete;
  LockedPtrConst& operator=(LockedPtrConst const&) = delete;

      LockedPtrConst(const T* obj, std::shared_mutex* obj_mutex):
        ref_(obj), mutex_(obj_mutex) {
    mutex_->lock_shared();
  }

      ~LockedPtrConst() {
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

  const T * get() const {
    return ref_;
  }

  private:
  const T* ref_ = nullptr;
  std::shared_mutex* mutex_ = nullptr;
};



}
