/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <map>
#include <functional>

#include "compatibility.hpp"
#include "poller.hpp"

namespace zmqpp
{

    class socket;
    typedef socket socket_t;

    /**
     * Reactor object that helps to manage multiple socket by calling a user-defined handler for each socket
     * when a watched event occurs.
     *
     * It uses zmq::poller as the underlying polling mechanism.
     */
    class reactor
    {
    public:
        typedef std::function<void (void) > Callable;
        typedef std::pair<zmq_pollitem_t, Callable> PollItemCallablePair;
        /**
         * Construct an empty polling model.
         */
        reactor();

        /**
         * Cleanup reactor.
         *
         * Any sockets will need to be closed separately.
         */
        ~reactor();
	
        /**
         * Add a socket to the reactor, providing a handler that will be called when the monitored events occur.
         *
         * \param socket the socket to monitor.
         * \param callable the function that will be called by the reactor when a registered event occurs on socket.
         * \param event the event flags to monitor on the socket.
         */
        void add(socket_t& socket, Callable callable, short const event = poller::poll_in);

        /*!
         * Add a standard socket to the reactor, providing a handler that will be called when the monitored events occur.
         *
         * \param descriptor the standard socket to monitor (SOCKET under Windows, a file descriptor otherwise).
         * \param callable the function that will be called by the reactor when a registered event occurs on fd.
         * \param event the event flags to monitor.
         */
        void add(raw_socket_t const descriptor, Callable callable, short const event = poller::poll_in | poller::poll_error);

        /**
         * Check if we are monitoring a given socket with this reactor.
         *
         * \param socket the socket to check.
         * \return true if it is there.
         */
        bool has(socket_t const& socket);

        /**
         * Check if we are monitoring a given standard socket with this reactor.
         *
         * \param descriptor the raw socket to check (SOCKET under Windows, a file descriptor otherwise).
         * \return true if it is there.
         */
        bool has(raw_socket_t const descriptor);

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
         * Update the monitored event flags for a given socket.
         *
         * \param socket the socket to update event flags.
         * \param event the event flags to monitor on the socket.
         */
        void check_for(socket_t const& socket, short const event);

        /*!
         * Update the monitored event flags for a given standard socket.
         *
         * \param descriptor the raw socket to update event flags (SOCKET under Windows, a file descriptor otherwise).
         * \param event the event flags to monitor on the socket.
         */
        void check_for(raw_socket_t const descriptor, short const event);

        /**
         * Poll for monitored events and call associated handler when needed.
         *
         * By default this method will block forever or until at least one of the monitored
         * sockets or file descriptors has events.
         *
         * If a timeout is set and was reached then this function returns false.
         *
         * \param timeout milliseconds to timeout.
         * \return true if there is an event..
         */
        bool poll(long timeout = poller::wait_forever);

        /**
         * Get the event flags triggered for a socket.
         *
         * \param socket the socket to get triggered event flags for.
         * \return the event flags.
         */
        short events(socket_t const& socket) const;

        /**
         * Get the event flags triggered for a standard socket.
         *
         * \param descriptor the raw socket to get triggered event flags for (SOCKET under Windows, a file descriptor otherwise).
         * \return the event flags.
         */
        short events(raw_socket_t const descriptor) const;


        /**
         * Get a reference to the underlying poller object used by the reactor.
         * Not sure this is useful.
         */
        poller &get_poller();

        /**
         * Get a reference to the underlying poller object used by the reactor (const version).
         * Not sure this is useful either.
         */
        const poller &get_poller() const;

    protected:
        void add(const zmq_pollitem_t &item, Callable callable);

    private:
        std::vector<PollItemCallablePair> items_;
        std::vector<const socket_t *> sockRemoveLater_;
        std::vector<raw_socket_t> fdRemoveLater_;
      
      /**
       * Flush the fdRemoveLater_ and sockRemoveLater_ vector, effectively removing
       * the item for the reactor and poller.
       */
      void flush_remove_later();

      poller poller_;
      bool dispatching_;
    };

}
