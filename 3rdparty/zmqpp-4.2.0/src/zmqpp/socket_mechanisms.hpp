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
 * \date   13 Aug 2014
 * \author Ben Gray (\@benjamg)
 */

#ifndef ZMQPP_SOCKET_MECHANISMS_HPP_
#define ZMQPP_SOCKET_MECHANISMS_HPP_

namespace zmqpp
{

/*!
 * \brief Socket security mechanisms allowed by zmq
 *
 * Each is designed for a different use and has different limitations.
 */
ZMQPP_COMPARABLE_ENUM socket_security {
#if (ZMQ_VERSION_MAJOR >= 4)
	/*!
	 * The NULL mechanism is defined by the ZMTP 3.0 specification:
	 * http://rfc.zeromq.org/spec:23. This is the default security mechanism
	 * for new ZeroMQ sockets
	 */
	none      = ZMQ_NULL,

	/*!
	 * The PLAIN mechanism defines a simple username/password mechanism that
	 * lets a server authenticate a client. PLAIN makes no attempt at security
	 * or confidentiality. It is intended for use on internal networks where
	 * security requirements are low. The PLAIN mechanism is defined by this
	 * document: http://rfc.zeromq.org/spec:24
	 */
	plain     = ZMQ_PLAIN,

	/*!
	 * The CURVE mechanism defines a mechanism for secure authentication and
	 * confidentiality for communications between a client and a server. CURVE
	 * is intended for use on public networks. The CURVE mechanism is defined
	 * by this document: http://rfc.zeromq.org/spec:25
	 */
	curve     = ZMQ_CURVE,
#endif
};

}

#endif /* ZMQPP_SOCKET_TYPES_HPP_ */
