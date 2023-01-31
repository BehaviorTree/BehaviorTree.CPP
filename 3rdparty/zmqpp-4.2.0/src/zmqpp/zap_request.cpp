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
 * \date   25 Nov 2014
 * \author Prem Shankar Kumar (\@meprem)
 */

#include "zap_request.hpp"
#include "message.hpp"
#include "socket.hpp"
#include "z85.hpp"
#include "byte_ordering.hpp"
#include <unordered_map>

#ifndef _WIN32
#include <netinet/in.h>
#endif

#if (ZMQ_VERSION_MAJOR > 3)

namespace zmqpp
{

/*!
 * Receive a ZAP valid request from the handler socket
 */
zap_request::zap_request(socket& handler, bool logging) :
  zap_socket(handler),
  verbose(logging)
{
    message msg;
    zap_socket.receive(msg);

    if(0 == msg.parts())
        return;     // Interrupted

    // Get all standard frames off the handler socket
    version   = msg.get(0);
    sequence  = msg.get(1);
    domain    = msg.get(2);
    address   = msg.get(3);
    identity  = msg.get(4);
    mechanism = msg.get(5); 

    // If the version is wrong, we're linked with a bogus libzmq, so die
    assert(version == "1.0");

    // Get mechanism-specific frames
    if("PLAIN" == mechanism) {
        username = msg.get(6); 
        password = msg.get(7);             

    } else if("CURVE" == mechanism) {
        client_key = z85::encode(msg.get(6)); 

    } else if("GSSAPI" == mechanism) {
        principal = msg.get(6);
    }

    if (verbose) {
        std::cout << "auth: ZAP request mechanism=" << mechanism 
            <<" ipaddress=" << address << std::endl;
    }
}

/*! 
 * Send a ZAP reply to the handler socket
 */
void zap_request::reply(const std::string &status_code, const std::string &status_text,
            const std::string &user_id, const std::unordered_map<std::string, std::string> &metadata_pairs)
{
    if (verbose)
    {
        std::cout << "auth: ZAP reply status_code=" << status_code
                << " status_text=" << status_text <<
                " user_id=" << user_id << std::endl;
    }

    message reply;
    reply.push_back(version);
    reply.push_back(sequence);
    reply.push_back(status_code);
    reply.push_back(status_text);
    reply.push_back(user_id);
    std::vector<uint8_t> md = serialize_metadata(metadata_pairs);
    reply.push_back(md.data(), md.size());

    zap_socket.send(reply);
}

std::vector<std::uint8_t> zap_request::serialize_metadata(
        const std::unordered_map<std::string, std::string> &metadata_pairs)
{
    std::vector<uint8_t> metadata;

    for (const auto &pair : metadata_pairs) {
        // name length (1 OCTET)
        assert(pair.first.length() < 256);
        metadata.push_back(static_cast<uint8_t>(pair.first.length()));

        // name
        std::copy(pair.first.begin(), pair.first.end(), std::back_inserter(metadata));

        // value length (4 OCTETs in network byte order)
        assert(pair.second.length() <= UINT32_MAX); // hm, really?
        auto value_length = HOST_TO_BIG_ENDIAN_32(static_cast<uint32_t>(pair.second.length()));
        std::copy(reinterpret_cast<uint8_t*>(&value_length),
                  reinterpret_cast<uint8_t*>(&value_length) + 4,
                  std::back_inserter(metadata));

        // value
        std::copy(pair.second.begin(), pair.second.end(), std::back_inserter(metadata));
    }

    return metadata;
}

}

#endif
