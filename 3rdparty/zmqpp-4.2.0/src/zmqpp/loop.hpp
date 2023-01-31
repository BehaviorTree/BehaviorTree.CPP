/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#pragma once

#include <tuple>
#include <vector>
#include <list>
#include <chrono>
#include <functional>
#include <memory>

#include "compatibility.hpp"
#include "poller.hpp"

namespace zmqpp
{

    class socket;
    typedef socket socket_t;

    /**
     * Loop object that helps to manage multiple socket by calling a user-defined handler for each socket
     * when a watched event occurs.
     * Calls assigned user-defined handler for timed events - repeaded and one-shot.
     *
     * It uses zmq::poller as the underlying polling mechanism.
     */
    class loop
    {
    public:
        /**
         * Type used to identify created timers withing loop
         */
        typedef void * timer_id_t;
        typedef std::function<bool (void) > Callable;

        /**
         * Construct an empty polling model.
         */
        loop();

        /**
         * Cleanup reactor.
         *
         * Any sockets will need to be closed separately.
         */
        virtual ~loop();

        /**
         * Add a socket to the loop, providing a handler that will be called when the monitored events occur.
         *
         * \param socket the socket to monitor.
         * \param callable the function that will be called by the loop when a registered event occurs on socket.
         * \param event the event flags to monitor on the socket.
         */
        void add(socket_t& socket, Callable callable, short const event = poller::poll_in);

        /*!
         * Add a standard socket to the loop, providing a handler that will be called when the monitored events occur.
         *
         * \param descriptor the standard socket to monitor (SOCKET under Windows, a file descriptor otherwise).
         * \param callable the function that will be called by the loop when a registered event occurs on fd.
         * \param event the event flags to monitor.
         */
        void add(raw_socket_t const descriptor, Callable callable, short const event = poller::poll_in | poller::poll_error);

        /**
         * Add a timed event to the loop, providing a handler that will be called when timer fires.
         *
         * \param delay time after which handler will be executed.
         * \param times how many times should timer be reneved - 0 for infinte ammount.
         * \param callable the function that will be called by the loop after delay.
         */
        timer_id_t add(std::chrono::milliseconds delay, size_t times, Callable callable);

        /**
         * Reset timer in the loop, it will start counting delay time again. Times argument is preserved.
         *
         * \param timer identifier in the loop.
         */
        void reset(timer_id_t const timer);

        /**
         * Remove timer event from the loop.
         *
         * \param timer identifier in the loop.
         */
        void remove(timer_id_t const timer);

        /**
         * Stop monitoring a socket.
         *
         * \param socket the socket to stop monitoring.
         */
        void remove(socket_t const& socket);

        /**
         * Stop monitoring a standard socket.
         *
         * \param descriptor the standard socket to stop monitoring.
         */
        void remove(raw_socket_t const descriptor);

        /**
         * Starts loop. It will block until one of handlers returns false.
         */
        void start();

    private:
        struct timer_t {
            size_t times;
            std::chrono::milliseconds delay;
            std::chrono::steady_clock::time_point when;

            timer_t(size_t times, std::chrono::milliseconds delay);

            void reset();
            void update();
        };

        typedef std::pair<zmq_pollitem_t, Callable> PollItemCallablePair;
        typedef std::pair<std::unique_ptr<timer_t>, Callable> TimerItemCallablePair;
        static bool TimerItemCallablePairComp(const TimerItemCallablePair &lhs, const TimerItemCallablePair &rhs);

        std::vector<PollItemCallablePair> items_;
        std::list<TimerItemCallablePair> timers_;
        std::vector<const socket_t *> sockRemoveLater_;
        std::vector<raw_socket_t> fdRemoveLater_;
        std::vector<timer_id_t> timerRemoveLater_;


        void add(const zmq_pollitem_t &item, Callable callable);
        void add(std::unique_ptr<timer_t>, Callable callable);

        bool start_handle_timers();
        bool start_handle_poller();

        /**
        * Flush the fdRemoveLater_ and sockRemoveLater_ vector, effectively removing
        * the item for the reactor and poller.
        */
        void flush_remove_later();

        /**
        * Calculate min time to wait in poller.
        */
        long tickless();

        poller poller_;
        bool dispatching_;
        bool rebuild_poller_;
    };

}
