/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#pragma once

#include "socket.hpp"

namespace zmqpp
{
  /**
   * Provide a simple, non-steerable proxy that will bidirectionally
   * forward traffic between socket A and B.
   *
   * If a *capture socket* is specified, the proxy shall send all messages,
   * received on both frontend and backend, to the capture socket.
   * The capture socket should be a ZMQ_PUB, ZMQ_DEALER, ZMQ_PUSH, or ZMQ_PAIR
   * socket.
   *
   * @note This is wrapper around `zmq_proxy()`.
   */
  class ZMQPP_EXPORT proxy
  {
  public:
    /**
     * Construct a proxy that will forward traffic from
     * A to B and from B to A.
     */
    proxy(socket &sa, socket &sb);

    /**
     * Construct a proxy that will forward traffic from A to B
     * and from B to A as-well as sending a copy of all message to `capture`
     */
    proxy(socket &sa, socket &sb, socket &capture);
  };
}
