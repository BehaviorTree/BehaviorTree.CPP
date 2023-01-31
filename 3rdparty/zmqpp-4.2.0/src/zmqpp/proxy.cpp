/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#include "proxy.hpp"

zmqpp::proxy::proxy(socket &sa, socket &sb)
{
  zmq_proxy(static_cast<void *>(sa), static_cast<void *>(sb), nullptr);
}

zmqpp::proxy::proxy(zmqpp::socket &sa, zmqpp::socket &sb,
                    zmqpp::socket &capture)
{
  zmq_proxy(static_cast<void *>(sa), static_cast<void *>(sb),
            static_cast<void *>(capture));
}
