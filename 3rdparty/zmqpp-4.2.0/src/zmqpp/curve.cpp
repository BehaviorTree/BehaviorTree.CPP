/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 *  Created on: 13 Aug 2014
 *      Author: Ben Gray (@benjamg)
 */

#include <vector>

#include <zmq_utils.h>

#include "compatibility.hpp"

#include "curve.hpp"
#include "exception.hpp"

namespace zmqpp { namespace curve {

#if (ZMQ_VERSION_MAJOR >= 4)
keypair generate_keypair()
{
	char public_key [41];
	char secret_key [41];

	int result = zmq_curve_keypair( public_key, secret_key );
	if (0 != result)
	{
		throw zmq_internal_exception();
	}

	return keypair{ public_key, secret_key };
}
#endif

} // end curve

}
