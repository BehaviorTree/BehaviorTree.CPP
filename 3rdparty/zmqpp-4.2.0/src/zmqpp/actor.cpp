/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 * File:   actor.cpp
 * Author: xaqq
 *
 * Created on May 20, 2014, 10:51 PM
 */

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "actor.hpp"
#include "socket.hpp"
#include "message.hpp"
#include "exception.hpp"
#include "context.hpp"

zmqpp::context zmqpp::actor::actor_pipe_ctx_;

namespace zmqpp
{

  actor::actor(ActorStartRoutine routine)
      : parent_pipe_(nullptr)
      , child_pipe_(nullptr)
      , stopped_(false)
  {
    std::string pipe_endpoint;

    parent_pipe_  = new socket(actor_pipe_ctx_, socket_type::pair);
    pipe_endpoint = bind_parent();

    child_pipe_ = new socket(actor_pipe_ctx_, socket_type::pair);
    child_pipe_->connect(pipe_endpoint);

    std::thread t(&actor::start_routine, this, child_pipe_, routine);
    t.detach();

    signal sig;
    sig = parent_pipe_->wait();
    assert(sig == signal::ok || sig == signal::ko);
    if (sig == signal::ko)
    {
      delete parent_pipe_;
      std::lock_guard<std::mutex> lg(mutex_);
      if (eptr_)
      {
        try
        {
          std::rethrow_exception(eptr_);
        }
        catch (std::exception &e)
        {
          std::throw_with_nested(actor_initialization_exception());
        }
      }
      else
      {
        throw actor_initialization_exception();
      }
    }
  }

  actor::actor(actor &&o)
  {
    *this = std::move(o);
  }

  actor &actor::operator=(actor &&o)
  {
    parent_pipe_   = o.parent_pipe_;
    stopped_       = o.stopped_;
    retval_        = o.retval_;
    o.parent_pipe_ = nullptr;

    return *this;
  }

  actor::~actor()
  {
    stop(true);
    delete parent_pipe_;
  }

  bool actor::stop(bool block /* = false */)
  {
    if (!parent_pipe_)
      return false;

    parent_pipe_->send(signal::stop, true);
    if (!block)
    {
      return true;
    }
    else
    {
      if (stopped_)
        return retval_;

      // wait for a response from the child. Either it successfully shutdown, or
      // not.
      signal sig = parent_pipe_->wait();
      stopped_ = true;
      assert(sig == signal::ok || sig == signal::ko);
      return (retval_ = (sig == signal::ok));
    }
  }

  void actor::start_routine(socket *child_pipe, ActorStartRoutine routine)
  {
    try
    {
      if (routine(child_pipe))
        child_pipe->send(signal::ok);
      else
        child_pipe->send(signal::ko);
    }
    catch (const std::exception &)
    {
      std::lock_guard<std::mutex> lg(mutex_);
      eptr_ = std::current_exception();
      child_pipe->send(signal::ko);
    }


    delete child_pipe;
  }

  socket *actor::pipe()
  {
    return parent_pipe_;
  }

  const socket *actor::pipe() const
  {
    return parent_pipe_;
  }

  std::string actor::bind_parent()
  {
    std::string base_endpoint =
        "inproc://zmqpp::actor::" +
        std::to_string(reinterpret_cast<ptrdiff_t>(this));

    for (;;)
    {
      try
      {
        std::string endpoint = base_endpoint + std::to_string(std::rand());
        parent_pipe_->bind(endpoint);
        return endpoint;
      }
      catch (zmq_internal_exception const &)
      {
        // endpoint already taken.
      }
    }
  }
}
