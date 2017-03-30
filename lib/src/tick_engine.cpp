#include <tick_engine.h>

TickEngine::TickEngine(int initial_value)
{
    value_ = initial_value;
}

TickEngine::~TickEngine() {}

void TickEngine::Wait()
{
    // Lock acquire (need a unique lock for the condition variable usage)
    std::unique_lock<std::mutex> UniqueLock(mutex_);

    // If the state is 0 then we have to wait for a signal
    if (value_ == 0)
        condition_variable_.wait(UniqueLock);

    // Once here we decrement the state
    value_--;
}

void TickEngine::Tick()
{
    // Lock acquire
    std::lock_guard<std::mutex> LockGuard(mutex_);

    // State increment
    value_++;

    // Notification
    condition_variable_.notify_all();
}
