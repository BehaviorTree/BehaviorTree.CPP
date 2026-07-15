#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

namespace BT::details
{

/**
 * @brief Synchronization gate used to coordinate callbacks with object teardown.
 *
 * Callbacks enter the gate with tryStart() and must signal completion when done
 * (use Guard for RAII). During teardown, call close() to reject new callbacks
 * and closeAndDrain() to block until the callbacks still running have completed.
 */
class CallbackGate
{
public:
  using Ptr = std::shared_ptr<CallbackGate>;

  /// RAII helper that signals the completion of a callback entered with tryStart()
  class Guard
  {
  public:
    explicit Guard(Ptr gate) : gate_(std::move(gate))
    {}

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
    Guard(Guard&&) = delete;
    Guard& operator=(Guard&&) = delete;

    ~Guard()
    {
      gate_->completed();
    }

  private:
    Ptr gate_;
  };

  /// Returns false if the gate has been closed and the callback must not run.
  bool tryStart()
  {
    const std::lock_guard lk(mutex_);
    if(!accepting_callbacks_)
    {
      return false;
    }
    running_callbacks_++;
    return true;
  }

  /// Reject new callbacks, without waiting for the running ones.
  void close()
  {
    const std::lock_guard lk(mutex_);
    accepting_callbacks_ = false;
  }

  /// Reject new callbacks and wait until callbacks already running have completed.
  void closeAndDrain()
  {
    std::unique_lock lk(mutex_);
    accepting_callbacks_ = false;
    condition_variable_.wait(lk, [this] { return running_callbacks_ == 0; });
  }

private:
  void completed()
  {
    const std::lock_guard lk(mutex_);
    running_callbacks_--;
    if(running_callbacks_ == 0 && !accepting_callbacks_)
    {
      condition_variable_.notify_all();
    }
  }

  std::mutex mutex_;
  std::condition_variable condition_variable_;
  bool accepting_callbacks_ = true;
  size_t running_callbacks_ = 0;
};

}  // namespace BT::details
