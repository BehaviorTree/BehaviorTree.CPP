#include <NodeSemaphore.h>

NodeSemaphore::NodeSemaphore(int InitialValue)
{
    Value = InitialValue;
}

NodeSemaphore::~NodeSemaphore() {}

void NodeSemaphore::Wait()
{
    // Lock acquire (need a unique lock for the condition variable usage)
    boost::unique_lock<boost::mutex> UniqueLock(Mutex);

    // If the state is 0 then we have to wait for a signal
    if (Value == 0)
        ConditionVariable.wait(UniqueLock);

    // Once here we decrement the state
    Value--;
}

void NodeSemaphore::Signal()
{
    // Lock acquire
    boost::lock_guard<boost::mutex> LockGuard(Mutex);

    // State increment
    Value++;

    // Notification
    ConditionVariable.notify_all();
}
