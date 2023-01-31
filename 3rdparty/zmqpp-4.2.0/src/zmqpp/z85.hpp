/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#pragma once
#include <string>
#include <vector>
#include <zmq.h>

namespace zmqpp
{
#if (ZMQ_VERSION_MAJOR >= 4)
  /**
  * Provide z85 encoding and decoding facilities.
  */
  namespace z85
  {
    /**
     * Encode a binary string into a string using Z85 representation.
     * @param raw_data the binary string to be encoded.
     * @return the encoded string.
     * See ZMQ RFC 32;
     */
    std::string encode(const std::string &raw_data);

    /**
     * Encode a binary blob into a string using Z85 representation.
     * @param data pointer to raw data to be encoded.
     * @param size the size of the data to be encoded.
     * @return the encoded string.
     * See ZMQ RFC 32;
     */
    std::string encode(const uint8_t *data, size_t size);

    /**
     * Decode a Z85 encoded string into a binary blob represented as a vector.
     * @param string the string to be decoded.
     * @return a vector of uint8_t: the binary block after string decoding.
     */
    std::vector<uint8_t> decode(const std::string &string);
  }
#endif

}
