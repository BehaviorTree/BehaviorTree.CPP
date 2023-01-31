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
 * \date   27 Sep 2016
 * \author Daniel Underwood (@danielunderwood)
 */

#pragma once

#if defined( __APPLE__) // Byte ordering on OS X

#include <libkern/OSByteOrder.h>
#define HOST_TO_BIG_ENDIAN_32(x) OSSwapHostToBigInt32(x)

#elif defined(_WIN32) // Byte ordering on Windows

#if BYTE_ORDER == LITTLE_ENDIAN
#include <winsock2.h>
#define HOST_TO_BIG_ENDIAN_32(x) htonl(x)

#elif BYTE_ORDER == BIG_ENDIAN
#define HOST_TO_BIG_ENDIAN_32(x) (x)

#endif

#else // Let htobe32 be the default function

#include <endian.h>
#define HOST_TO_BIG_ENDIAN_32(x) htobe32(x)

#endif

// Cause a compiler error if the platform is not supported
#ifndef HOST_TO_BIG_ENDIAN_32
#error Platform not supported in byte_ordering.hpp
#endif
