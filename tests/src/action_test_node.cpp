/* Copyright (C) 2015-2017 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "action_test_node.h"
#include <string>
#include <ctime>
#include <chrono>
#include <time.h>
#include <ratio>
#include <iomanip>
using namespace std::chrono;

BT::AsyncActionTest::AsyncActionTest(const std::string& name, BT::Duration deadline_ms) :
    AsyncActionNode(name, {})
{
    boolean_value_ = true;
    time_ = deadline_ms;
    stop_loop_ = false;
    tick_count_ = 0;
}

BT::AsyncActionTest::~AsyncActionTest()
{
    halt();
}

BT::NodeStatus BT::AsyncActionTest::tick()
{

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

//    std::cout << name() << " STARTED at"  << t2.time_since_epoch().count()  << std::endl;

    setStartTimePoint(t2);
    setStatus(NodeStatus::RUNNING);
    using std::chrono::high_resolution_clock;
    tick_count_++;
    stop_loop_ = false;
    auto initial_time = high_resolution_clock::now();


    while (!stop_loop_ && high_resolution_clock::now() < initial_time + time_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << name() << " running"  << std::endl;

    }
//    std::cout << name() << " STARTED at"  << high_resolution_clock::now().time_since_epoch().count()  << std::endl;

    setStopTimePoint(high_resolution_clock::now());

    if (!stop_loop_)
    {
        return boolean_value_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }
    else
    {
        return NodeStatus::IDLE;
    }
}



void BT::AsyncActionTest::halt()
{
    high_resolution_clock::time_point t2 = high_resolution_clock::now();

//    std::cout << name() << " HALTED at "  << t2.time_since_epoch().count() << std::endl;

    stop_loop_ = true;
 NodeStatus node_status;
    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

       node_status = status();
    } while (node_status == NodeStatus::RUNNING);

}

void BT::AsyncActionTest::setTime(BT::Duration time)
{
    time_ = time;
}

void BT::AsyncActionTest::setBoolean(bool boolean_value)
{
    boolean_value_ = boolean_value;
}


void BT::AsyncActionTest::setStartTimePoint(std::chrono::high_resolution_clock::time_point now)
{
    std::lock_guard<std::mutex> lock(start_time_mutex_);

    start_time_ = now;
}

std::chrono::high_resolution_clock::time_point BT::AsyncActionTest::startTimePoint() const
{
    std::lock_guard<std::mutex> lock(start_time_mutex_);

    return start_time_;
}


void BT::AsyncActionTest::setStopTimePoint(std::chrono::high_resolution_clock::time_point now)
{
    std::lock_guard<std::mutex> lock(stop_time_mutex_);

    stop_time_ = now;
}

std::chrono::high_resolution_clock::time_point BT::AsyncActionTest::stopTimePoint() const
{
    std::lock_guard<std::mutex> lock(stop_time_mutex_);

    return stop_time_;
}


//----------------------------------------------

BT::SyncActionTest::SyncActionTest(const std::string& name) :
    SyncActionNode(name, {})
{
    tick_count_ = 0;
    boolean_value_ = true;
}

BT::NodeStatus BT::SyncActionTest::tick()
{
    tick_count_++;
    return boolean_value_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

void BT::SyncActionTest::setBoolean(bool boolean_value)
{
    boolean_value_ = boolean_value;
}
