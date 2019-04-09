#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include <ctime>
#include <chrono>
#include "behaviortree_cpp/action_node.h"
#include <mutex>

namespace BT
{
class SyncActionTest : public SyncActionNode
{
  public:
    SyncActionTest(const std::string& name);

    BT::NodeStatus tick() override;

    void setBoolean(bool boolean_value);

    int tickCount() const
    {
        return tick_count_;
    }

    void resetTicks()
    {
        tick_count_ = 0;
    }

  private:
    bool boolean_value_;
    int tick_count_;
};

class AsyncActionTest : public AsyncActionNode
{
  public:
    AsyncActionTest(const std::string& name, BT::Duration deadline_ms);

    ~AsyncActionTest();

    // The method that is going to be executed by the thread
    BT::NodeStatus tick() override;

    void setTime(BT::Duration time);

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void setBoolean(bool boolean_value);

    int tickCount() const
    {
        return tick_count_;
    }

    void resetTicks()
    {
        tick_count_ = 0;
    }

    void setStartTimePoint(std::chrono::high_resolution_clock::time_point now)
    {
        std::lock_guard<std::mutex> lock(start_time_mutex_);
//       std::cout << this->name() << " Setting Start Time: " << now.time_since_epoch().count() << std::endl;

        start_time_ = now;
    }

    std::chrono::high_resolution_clock::time_point startTimePoint()
    {
        std::lock_guard<std::mutex> lock(start_time_mutex_);

        return start_time_;
    }


    void setStopTimePoint(std::chrono::high_resolution_clock::time_point now)
    {
        std::lock_guard<std::mutex> lock(stop_time_mutex_);

//        std::cout << this->name() << " Setting Stop Time: " << now.time_since_epoch().count()  << std::endl;

        stop_time_ = now;
    }

    std::chrono::high_resolution_clock::time_point stopTimePoint()
    {
        std::lock_guard<std::mutex> lock(stop_time_mutex_);

        return stop_time_;
    }


private:
    // using atomic because these variables might be accessed from different threads
    BT::Duration time_;
    std::atomic_bool boolean_value_;
    std::atomic<int> tick_count_;
    std::atomic_bool stop_loop_;
    std::chrono::high_resolution_clock::time_point start_time_, stop_time_;

    std::mutex start_time_mutex_, stop_time_mutex_;

};
}

#endif
