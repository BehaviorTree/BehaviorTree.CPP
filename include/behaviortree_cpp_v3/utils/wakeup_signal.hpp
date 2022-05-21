/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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
        ready_ = false;
        return cv_.wait_for(lk, tm, [this]{ return ready_; });
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
