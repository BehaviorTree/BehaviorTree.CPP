/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/**
 * \file
 *
 * \date   9 Aug 2011
 * \author Ben Gray (\@benjamg)
 *
 * License: http://www.opensource.org/licenses/MIT
 *
 * Copyright (C) 2011 by Ben Gray
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ZMQPP_ZMQPP_HPP_
#define ZMQPP_ZMQPP_HPP_

/**
 * \def ZMQPP_VERSION_MAJOR
 * zmqpp major version number
 */
#define	ZMQPP_VERSION_MAJOR 4

/**
 * \def ZMQPP_VERSION_MINOR
 * zmqpp minor version number
 */
#define	ZMQPP_VERSION_MINOR 1
/**
 * \def ZMQPP_VERSION_REVISION
 * zmqpp version revision number
 */
#define	ZMQPP_VERSION_REVISION 1

#include <zmq.h>

#include "compatibility.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "message.hpp"
#include "poller.hpp"
#include "socket.hpp"
#include "actor.hpp"
#include "reactor.hpp"
#include "loop.hpp"
#include "zap_request.hpp"
#include "auth.hpp"

/*!
 * \brief C++ wrapper around zmq
 *
 * All zmq++ / zmqpp functions, constants and classes live within this namespace,
 */
namespace zmqpp
{

/*!
 * Returns the current major.minor.revision version number as a string.
 *
 * \return string version number.
 */
ZMQPP_EXPORT std::string version();

/*!
 * Retrieve the parts of the zmqpp version number.
 *
 * Set the three parameters to values representing the zmqpp version number.
 * These values are generated at library compile time.
 *
 * \param major an unsigned 8 bit reference to set to the major version.
 * \param minor an unsigned 8 bit reference to set to the minor version.
 * \param revision an unsigned 8 bit reference to set the current revision.
 */
ZMQPP_EXPORT void version(uint8_t& major, uint8_t& minor, uint8_t& revision);

/*!
 * Retrieve the parts of the 0mq version this library was built against.
 *
 * Because sections of the library are optionally compiled in or ignored
 * depending on the version of 0mq it was compiled against this method is
 * provided to allow sanity checking for usage.
 *
 * Set the three parameters to values representing the 0mq version number.
 * These values are generated at library compile time.
 *
 * \param major an unsigned 8 bit reference to set to the major version.
 * \param minor an unsigned 8 bit reference to set to the minor version.
 * \param revision an unsigned 8 bit reference to set the current revision.
 */
ZMQPP_EXPORT void zmq_version(uint8_t& major, uint8_t& minor, uint8_t& patch);

#if (ZMQ_VERSION_MAJOR > 4) || ((ZMQ_VERSION_MAJOR == 4) && (ZMQ_VERSION_MINOR >= 1))
/*!
 * Check for support in the underlaying 0mq library.
 *
 * This is a simple wrapper around the zmq_has capbaility check.
 * Please see the 0mq documentation for a list of valid capabiliy strings.
 */
ZMQPP_EXPORT bool has_capability(std::string const& capability);

/**
 * The following methods are helper functions for the known capabilies
 * that the underlaying 0mq service supports.
 */

/* Protcols */
inline bool has_protocol_ipc() { return has_capability("ipc"); }
inline bool has_protocol_pgm() { return has_capability("pgm"); }
inline bool has_protocol_tipc() { return has_capability("tipc"); }
inline bool has_protocol_norm() { return has_capability("norm"); }

/* Security Mechanisms */
inline bool has_security_curve() { return has_capability("curve"); }
inline bool has_security_gssapi() { return has_capability("gssapi"); }

/*!
 * Check if the underlaying 0mq library was built with the draft api.
 *
 * \returns true if it was
 */
inline bool is_draft_api() { return has_capability("draft"); }
#endif

typedef context     context_t;   /*!< \brief context type */
typedef std::string endpoint_t;  /*!< \brief endpoint type */
typedef message     message_t;   /*!< \brief message type */
typedef poller      poller_t;    /*!< \brief poller type */
typedef socket      socket_t;    /*!< \brief socket type */

}

#endif /* ZMQPP_ZMQPP_HPP_ */
