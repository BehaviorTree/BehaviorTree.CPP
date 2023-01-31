/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#include "proxy_steerable.hpp"

#if (ZMQ_VERSION_MAJOR >= 4)

zmqpp::proxy_steerable::proxy_steerable(
    zmqpp::socket &sa,
    zmqpp::socket &sb,
    zmqpp::socket &control )
{
  zmq_proxy_steerable(static_cast<void *>(sa), static_cast<void *>(sb), nullptr,
      static_cast<void *>(control));
}

zmqpp::proxy_steerable::proxy_steerable(
    zmqpp::socket &sa,
    zmqpp::socket &sb,
    zmqpp::socket &control,
    zmqpp::socket &capture )
{
  zmq_proxy_steerable(static_cast<void *>(sa), static_cast<void *>(sb),
      static_cast<void *>(capture), static_cast<void *>(control));
}

#endif

