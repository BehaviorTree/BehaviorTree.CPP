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
 */

#ifndef ZMQPP_EXCEPTION_HPP_
#define ZMQPP_EXCEPTION_HPP_

#include <stdexcept>
#include <string>

#include <zmq.h>

#include "compatibility.hpp"

namespace zmqpp
{

/** \todo Have a larger variety of exceptions with better state debug information */

/**
 * Represents the base zmqpp exception.
 *
 * All zmqpp runtime exceptions are children of this class.
 * The class itself does not provide any special access fields but it only
 * for convince when catching exceptions.
 *
 * The class extends std::runtime_error.
 *
 */
class ZMQPP_EXPORT exception : public std::runtime_error
{
public:
	/**
	 * Standard exception constructor.
	 *
	 * \param message a string representing the error message.
	 */
	exception(std::string const& message)
		: std::runtime_error(message)
	{ }
};

/**
 * Represents an attempt to use an invalid object.
 *
 * Objects may be invalid initially or after a shutdown or close.
 */
class ZMQPP_EXPORT invalid_instance : public exception
{
public:
	invalid_instance(std::string const& message)
		: exception(message)
	{ }
};

    /**
     * Represents a failed zmqpp::actor initialization.
     */
    class ZMQPP_EXPORT actor_initialization_exception : public exception
    {
    public:

	actor_initialization_exception() :
	exception("Actor Initialization Exception")
	{
	}

    };

  /**
   * Thrown when an error occurs while encoding or decoding to/from z85.
   * See ZMQ RFC 32
   */
  class ZMQPP_EXPORT z85_exception : public exception
  {
  public:
    z85_exception(const std::string &msg):
      exception(msg)
    {
    }
  };

/**
 * Represents internal zmq errors.
 *
 * Any error response from the zmq bindings will be wrapped in this error.
 *
 * The class provides access to the zmq error number via zmq_error().
 */
class ZMQPP_EXPORT zmq_internal_exception : public exception
{
public:
	/**
	 * Uses the zmq functions to pull out error messages and numbers.
	 */
	zmq_internal_exception()
		: exception(zmq_strerror(zmq_errno()))
		, _error(zmq_errno())
	{ }

	/**
	 * Retrieve the zmq error number associated with this exception.
	 * \return zmq error number
	 */
	int zmq_error() const { return _error; }

private:
	int _error;
};

}

#endif /* ZMQPP_EXCEPTION_HPP_ */
