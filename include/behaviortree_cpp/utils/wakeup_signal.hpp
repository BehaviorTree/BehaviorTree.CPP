#ifndef BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
#define BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP

#include <chrono>
#include <mutex>
#include <condition_variable>

namespace BT
{

class WakeUpSignal
{
public:
    /// Return true if the
    bool waitFor(std::chrono::system_clock::duration tm)
    {
        std::unique_lock<std::mutex> lk(mutex_);
        auto res = cv_.wait_for(lk, tm, [this]{
          return ready_;
        });
        ready_ = false;
        return res;
    }

    void emitSignal()
    {
       {
           std::lock_guard<std::mutex> lk(mutex_);
           ready_ = true;
       }
       cv_.notify_all();
    }

private:

    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_ = false;
};

}

#endif // BEHAVIORTREECORE_WAKEUP_SIGNAL_HPP
