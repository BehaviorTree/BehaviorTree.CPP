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
 * \date   3 Jul 2013
 * \author Ben Gray (\@benjamg)
 */

#ifndef ZMQPP_CONTEXT_OPTIONS_HPP_
#define ZMQPP_CONTEXT_OPTIONS_HPP_

namespace zmqpp
{

/** \todo Expand the information on the options to make it actually useful. */
/*!
 * \brief possible Context options in zmq
 */

ZMQPP_COMPARABLE_ENUM context_option {
	io_threads           = ZMQ_IO_THREADS,           /*!< I/O thread count */
	max_sockets          = ZMQ_MAX_SOCKETS,          /*!< Maximum supported sockets */
#if (ZMQ_VERSION_MAJOR >= 4)
#if (ZMQ_VERSION_MINOR >= 1)
	thread_sched_policy  = ZMQ_THREAD_SCHED_POLICY,  /*!< Scheduling policy for I/O threads */
	thread_priority      = ZMQ_THREAD_PRIORITY,      /*!< Scheduling priority for I/O threads */
#endif
	ipv6                 = ZMQ_IPV6                  /*!< Enable ipv6 for all new sockets */
#endif
};

}

#endif /* ZMQPP_CONTEXT_OPTIONS_HPP_ */
