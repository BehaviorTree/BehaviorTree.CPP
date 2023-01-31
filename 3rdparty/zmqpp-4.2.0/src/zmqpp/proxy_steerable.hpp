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
#if (ZMQ_VERSION_MAJOR >= 4)
  /**
   * Provide a steerable proxy that will bidirectionally
   * forward traffic between socket A and B.
   *
   * If *capture socket* is not null, the proxy shall send all messages,
   * received on both frontend and backend, to the capture socket.
   * The capture socket should be a ZMQ_PUB, ZMQ_DEALER, ZMQ_PUSH, or ZMQ_PAIR
   * socket.
   *
   * If the control socket is not NULL, the proxy supports control flow.
   * If PAUSE is received on this socket, the proxy suspends its activities.
   * If RESUME is received, it goes on.
   * If TERMINATE is received, it terminates smoothly.
   * 
   *
   * @note This is wrapper around `zmq_proxy_steerable()`.
   */
  class proxy_steerable
  {
  public:
    /**
     * Construct a proxy that will forward traffic from
     * A to B and from B to A and receive control messages in `control`
     */
    proxy_steerable(socket &sa, socket &sb, socket &control );

    /**
     * Construct a proxy that will forward traffic from
     * A to B and from B to A and receive control messages in `control` as well
     * as sending a copy of all messages to `capture`
     */
    proxy_steerable(socket &sa, socket &sb, socket &control, socket &capture );
  };
#endif

}
