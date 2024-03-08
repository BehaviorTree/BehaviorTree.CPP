#ifndef BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
#define BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP

#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace BT
{

class WakeUpSignal
{
public:
  /// Return true if the timeout was NOT reached and the
  /// signal was received.
  bool waitFor(std::chrono::microseconds usec)
  {
    std::unique_lock<std::mutex> lk(mutex_);
    auto res = cv_.wait_for(lk, usec, [this] { return ready_.load(); });
    ready_ = false;
    return res;
  }

  void emitSignal()
  {
    ready_ = true;
    cv_.notify_all();
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic_bool ready_ = false;
};

}  // namespace BT

#endif  // BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
