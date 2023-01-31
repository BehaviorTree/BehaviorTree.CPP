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

#ifndef ZMQPP_CURVE_HPP_
#define ZMQPP_CURVE_HPP_

#include <string>
#include <zmq.h>

namespace zmqpp {
/**
* ZMQ curve facilities.
*/
namespace curve
{

/**
* A pair of public and private key.
*/
struct keypair
{
	std::string public_key;
	std::string secret_key;
};

#if (ZMQ_VERSION_MAJOR >= 4)
keypair generate_keypair();
#endif

} }

#endif /* ZMQPP_CURVE_HPP_ */
