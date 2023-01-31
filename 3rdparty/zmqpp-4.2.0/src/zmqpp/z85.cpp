/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#include "z85.hpp"
#include <memory>
#include <zmq.h>
#include "exception.hpp"

using namespace zmqpp;

#if (ZMQ_VERSION_MAJOR >= 4)
std::string z85::encode(const std::string &raw_data)
{
  return encode(reinterpret_cast<const uint8_t*>(raw_data.c_str()), raw_data.size());
}

std::string z85::encode(const uint8_t *data, size_t size)
{
  std::unique_ptr<char[]> storage(new char[size * 5 / 4 + 1]);

  if (zmq_z85_encode(&storage[0], const_cast<uint8_t *>(data), size) == nullptr)
    {
      throw z85_exception("Failed to encode to z85");
    }
  return std::string(&storage[0]);
}

std::vector<uint8_t> z85::decode(const std::string &string)
{
  std::vector<uint8_t> dest(string.size() * 4 / 5);
  
  if (zmq_z85_decode(&dest[0], const_cast<char *>(string.c_str())) == nullptr)
  {
    throw z85_exception("Failed to decode from z85");
  }
  return dest;
}
#endif
