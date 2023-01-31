/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include "context.hpp"
#include "socket.hpp"

namespace zmqpp
{

  /**
   * An actor is a thread with a pair socket connected to its parent.
   * It aims to be similar to CMZQ's zactor.
   *
   * From the parent thread, instancing an actor will spawn a new thread, and
   * install
   * a pipe between those two threads.
   *	1. The parent's end of the pipe can be retrieved by calling pipe() on
   *       the actor.
   *	2. The child's end is passed as a parameter to the routine executed in
   *       the child's thread.
   *
   * You don't have to manage the 2 PAIR sockets. The parent's one will be
   * destroyed when the actor dies, and the child's end is taken care
   * of when the user routine ends.
   *
   * @note
   *  About user-supplied routine return value:
   *	1. If the supplied routine returns true, signal::ok will be send to the
   *       parent.
   *	2. If it returns false, signal::ko is send instead.
   *
   * @note There is a simple protocol between actor and parent to avoid
   *       synchronization problem:
   *	1. The actor constructor expect to receive either signal::ok or
   *       signal::ko before it returns.
   *	2. When sending signal::stop (actor destruction or by calling stop()),
   *       we expect a response: (either signal::ko or signal::ok).
   *       This response is used to determine if stop() will return true or
   *       false when in blocking mode.
   */
  class actor
  {
  public:
    /**
     * The user defined function type.
     */
    typedef std::function<bool(socket *pipe)> ActorStartRoutine;

    /**
     * Create a new actor. This will effectively create a new thread and runs
     * the user supplied routine.
     * The constructor expect a signal from the routine before returning.
     *
     * Expect to receive either signal::ko or signal::ok before returning.
     * If it receives signal::ko, it will throw.
     * @param routine to be executed.
     */
    actor(ActorStartRoutine routine);
    actor(const actor &) = delete;

    /**
     * Move constructor. The other actor will not be usable anymore.
     * Its pipe() method shall return null, and stop() will do nothing.
     */
    actor(actor &&o);

    /**
     * Move-assignment operator.
     * @see move constructor.
     */
    actor &operator=(actor &&o);

    virtual ~actor();

    /**
     * @return pointer to the parent's end of the pipe
     */
    socket *pipe();

    /**
     * @return const pointer to the parent's end of the pipe
     */
    const socket *pipe() const;

    /**
     * Sends signal::stop to the actor thread.
     * The actor thread shall stop as soon as possible.
     * The return value is only relevant when block is true. If block is false
     * (the default), this method
     * will return true.
     *
     * It is safe to call stop() multiple time (to ask the actor to shutdown,
     * and then a bit
     * later call stop(true) to make sure it is really stopped)
     *
     * @note calling this method on an "empty" actor (after it was moved) will
     * return
     * false and nothing will happen.
     * @param block whether or not we wait until the actor thread stops.
     * @return a boolean indicating whether or not the actor successfully
     * shutdown.
     *
     * @note Exception thrown during initialization (before sending ko/ok) are
     *       rethrown in the actor's constructor.
     */
    bool stop(bool block = false);

  private:
    /**
     * Call a user defined function and performs cleanup once it returns.
     * We use a copy of child_pipe_ here; this is to avoid a race condition
     * where the
     * destructor would be called before start_routine finishes but after it
     * sent signal::ok / signal::ko.
     * The actor object would be invalid (because already destroyed).
     *
     * @param child a copy of child_pipe_.
     * @param routine user routine that will be called
     */
    void start_routine(socket *child, ActorStartRoutine routine);

    /**
    * Bind the parent socket and return the endpoint used.
    * Since endpoint are generated and have to be tested for availability
    * this method is responsible for finding a valid endpoint to bind to.
    */
    std::string bind_parent();

    /**
     * The parent thread socket.
     * This socket will be closed and freed by the actor destructor.
     */
    socket *parent_pipe_;

    /**
     * The child end of the pipe.
     * It is closed and freed when the routine ran by the actor ends.
     */
    socket *child_pipe_;

    /**
     * This static, per process zmqpp::context, is used to connect PAIR socket
     * between Actor and their parent thread.
     */
    static context actor_pipe_ctx_;

    mutable std::mutex mutex_;

    std::exception_ptr eptr_;

    /**
     * Keeps track of the status of the actor thread.
     */
    bool stopped_;

    bool retval_;
  };
}
