/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 *  Created on: 20 Mar 2017
 *      Author: Jakub Piskorz (@piskorzj)
 */

#include "exception.hpp"
#include "socket.hpp"
#include "loop.hpp"
#include <algorithm>
#include <zmq.h>

namespace zmqpp
{
    loop::loop() :
    dispatching_(false),
    rebuild_poller_(false)
    {
    }

    loop::~loop()
    {
    }

    loop::timer_t::timer_t(size_t times, std::chrono::milliseconds delay) :
    times(times),
    delay(delay),
    when(std::chrono::steady_clock::now() + delay)
    {
    }

    void loop::timer_t::reset()
    {
         when = std::chrono::steady_clock::now() + delay;
    }

    void loop::timer_t::update()
    {
        when += delay;
    }

    void loop::add(socket& socket, Callable callable, short const event /* = POLL_IN */)
    {
        zmq_pollitem_t item{static_cast<void *> (socket), 0, event, 0};
        add(item, callable);
    }

    void loop::add(raw_socket_t const descriptor, Callable callable, short const event /* = POLL_IN */)
    {
        zmq_pollitem_t item{nullptr, descriptor, event, 0};
        add(item, callable);
    }

    void loop::add(const zmq_pollitem_t& item, Callable callable)
    {
        poller_.add(item);
        rebuild_poller_ = true;
        items_.push_back(std::make_pair(item, callable));
    }

    loop::timer_id_t loop::add(std::chrono::milliseconds delay, size_t times, Callable callable)
    {
        std::unique_ptr<timer_t> item(new timer_t(times, delay));
        timer_id_t id = item.get();
        add(std::move(item), callable);
        return id;
    }

    void loop::add(std::unique_ptr<timer_t> item, Callable callable)
    {
        timers_.push_back(std::make_pair(std::move(item), callable));
        timers_.sort(TimerItemCallablePairComp);
    }

    void loop::reset(timer_id_t const timer) {
        for(auto &item : timers_) {
            if(item.first.get() == timer) {
                item.first->reset();
                timers_.sort(TimerItemCallablePairComp);
                return;
            }
        }
    }

    void loop::remove(timer_id_t const timer)
    {
        if(dispatching_)
        {
            timerRemoveLater_.push_back(timer);
            return;
        }
        timers_.remove_if([&timer](const TimerItemCallablePair &pair) -> bool {
            return pair.first.get() == timer;
        });
    }

    void loop::remove(socket_t const& socket)
    {
        if (dispatching_)
        {
            rebuild_poller_ = true;
            sockRemoveLater_.push_back(&socket);
            return;
        }
        items_.erase(std::remove_if(items_.begin(), items_.end(), [&socket](const PollItemCallablePair & pair) -> bool
        {
            const zmq_pollitem_t &item = pair.first;
            if (nullptr != item.socket && item.socket == static_cast<void *> (socket))
            {
                return true;
            }
            return false;
        }), items_.end());
        poller_.remove(socket);
    }

    void loop::remove(raw_socket_t const descriptor)
    {
        if (dispatching_)
        {
            rebuild_poller_ = true;
            fdRemoveLater_.push_back(descriptor);
            return;
        }
        items_.erase(std::remove_if(items_.begin(), items_.end(), [descriptor](const PollItemCallablePair & pair) -> bool
        {
            const zmq_pollitem_t &item = pair.first;
            if (nullptr == item.socket && item.fd == descriptor)
            {
                return true;
            }
            return false;
        }), items_.end());
        poller_.remove(descriptor);
    }

    void loop::start()
    {
        while(1) {
            rebuild_poller_ = false;
            flush_remove_later();
            bool poll_rc = poller_.poll(tickless());

            dispatching_ = true;
            bool continue_looping = start_handle_timers();
            dispatching_ = false;

            if(!continue_looping)
                break;

            if(rebuild_poller_)
                continue;

            dispatching_ = true;
            if(poll_rc)
                continue_looping = start_handle_poller();
            dispatching_ = false;

            if(!continue_looping)
                break;
        }
        flush_remove_later();
    }

    bool loop::start_handle_timers()
    {
        std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        auto it = timers_.begin();
        while(it != timers_.end()) {
            if((*it).first->when < time_now) {
                bool timer_succedd = (*it).second();
                if((*it).first->times && --(*it).first->times == 0) {
                    it = timers_.erase(it);
                } else {
                    (*it).first->update();
                    ++it;
                }
                if(!timer_succedd)
                    return false;
            } else
                break;
        }
        timers_.sort(TimerItemCallablePairComp);
        return true;
    }

    bool loop::start_handle_poller()
    {
        for (const PollItemCallablePair &pair : items_)
        {
            const zmq_pollitem_t &pollitem = pair.first;

            if (poller_.has_input(pollitem) || poller_.has_error(pollitem) || poller_.has_output(pollitem))
                if(!pair.second())
                    return false;
        }
        return true;
    }

    void loop::flush_remove_later()
    {
        for (raw_socket_t fd : fdRemoveLater_)
            remove(fd);
        for (const socket_t *sock : sockRemoveLater_)
            remove(*sock);
        for (const timer_id_t & timer : timerRemoveLater_)
            remove(timer);
        fdRemoveLater_.clear();
        sockRemoveLater_.clear();
        timerRemoveLater_.clear();
    }

    long loop::tickless() {
        std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now() + std::chrono::hours(1);
        if(!timers_.empty() && timers_.front().first->when < tick)
            tick = timers_.front().first->when;
        long timeout = std::chrono::duration_cast<std::chrono::milliseconds>(tick - std::chrono::steady_clock::now()).count();
        if(timeout < 0)
            timeout = 0;
        return timeout;
    }

    bool loop::TimerItemCallablePairComp(const TimerItemCallablePair &lhs, const TimerItemCallablePair &rhs)
    {
        return lhs.first->when < rhs.first->when;
    }

}
