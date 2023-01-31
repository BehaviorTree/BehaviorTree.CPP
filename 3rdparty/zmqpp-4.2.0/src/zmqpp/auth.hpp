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

#ifndef ZMQPP_AUTH_HPP_
#define ZMQPP_AUTH_HPP_

#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "actor.hpp"
#include "poller.hpp"
#include "socket.hpp"
#include "context.hpp"
#include "zap_request.hpp"


// Authentication is something from zmq 4
#if (ZMQ_VERSION_MAJOR > 3)

namespace zmqpp
{

/**
 * auth - authentication for ZeroMQ security mechanisms
 * 
 * An auth actor takes over authentication for all incoming connections in
 * its context. You can whitelist or blacklist peers based on IP address,
 * and define policies for securing PLAIN, CURVE, and GSSAPI connections.
 *
 */
class auth 
{
public:
	/**
	 * Constructor. A auth actor takes over authentication for all incoming connections in
	 * its context. You can whitelist or blacklist peers based on IP address,
	 * and define policies for securing PLAIN, CURVE, and GSSAPI connections.
	 *
	 */
	auth(context& ctx);

	/**
	 * Destructor.
	 *
	 */
    	~auth();

	/**
	 * Allow (whitelist) a single IP address. For NULL, all clients from this
	 * address will be accepted. For PLAIN and CURVE, they will be allowed to
	 * continue with authentication. You can call this method multiple times
	 * to whitelist multiple IP addresses. If you whitelist a single address,
	 * any non-whitelisted addresses are treated as blacklisted.
	 *
	 */
    	void allow(const std::string &address);

    	/**
	 * Deny (blacklist) a single IP address. For all security mechanisms, this
	 * rejects the connection without any further authentication. Use either a
     	 * whitelist, or a blacklist, not not both. If you define both a whitelist
     	 * and a blacklist, only the whitelist takes effect.
	 *
	 */
    	void deny(const std::string &address);

    	/**
	 * Configure a ZAP domain. To cover all domains, use "*".
	 */
    	void configure_domain(const std::string &domain);

    	/**
	 * Configure PLAIN authentication. PLAIN authentication uses a plain-text 
	 * username and password.
	 *
	 */
    	void configure_plain(const std::string &username, const std::string &password);

    	/**
	 * Configure CURVE authentication. CURVE authentication uses client public keys. 
	 * This method can be called multiple times. To cover all domains, use "*". 
	 * To allow all client keys without checking, specify CURVE_ALLOW_ANY for the client_public_key.
	 *
	 */
    	void configure_curve(const std::string &client_public_key);

    	/**
	 * Configure GSSAPI authentication. GSSAPI authentication uses an underlying 
	 * mechanism (usually Kerberos) to establish a secure context and perform mutual 
	 * authentication.
	 *
	 */
    	void configure_gssapi();

    	/**
	 * Enable verbose tracing of commands and activity.
	 *
	 */
    	void set_verbose(bool verbose);

private:
	/**
	 * Handle an authentication command from calling application.
	 *
	 */
    	void handle_command(socket& pipe);

	/**
	 * Handle a PLAIN authentication request from libzmq core
	 *
	 * @param user_id store the user as the User-Id.
	 */
    	bool authenticate_plain(zap_request& request, std::string &user_id);

	/**
	 * Handle a CURVE authentication request from libzmq core
	 *
	 * @param user_id store the public key (z85 encoded) as the User-Id.
	 */
    	bool authenticate_curve(zap_request& request, std::string &user_id);

    	/**
	 * Handle a GSSAPI authentication request from libzmq core
	 *
	 */
    	bool authenticate_gssapi(zap_request& request);

    	/**
     	 * Authentication.
     	 *
     	 */
    	void authenticate(socket& sock);
    
    	std::shared_ptr<actor>    		 	authenticator;	// ZAP authentication actor
    	poller 	            			 	auth_poller;	 // Socket poller
    	std::unordered_set<std::string> 		whitelist;      // Whitelisted addresses
    	std::unordered_set<std::string> 		blacklist;      // Blacklisted addresses
    	std::unordered_map<std::string, std::string> 	passwords;      // PLAIN passwords, if loaded
    	std::unordered_set<std::string> 		client_keys;    // Client public keys
    	std::string 				 	domain;			// ZAP domain
    	bool                        curve_allow_any;      // CURVE allows arbitrary clients
    	bool 					 	terminated;     // Did caller ask us to quit?
    	bool 					 	verbose;        // Verbose logging enabled?

#	if defined(ZMQPP_NO_CONSTEXPR)
        static const char * const zap_endpoint_;
#	else
        constexpr static const char * const zap_endpoint_ = "inproc://zeromq.zap.01";
#	endif
  
    	// No copy - private and not implemented
    	auth(auth const&) ZMQPP_EXPLICITLY_DELETED;
    	auth& operator=(auth const&) NOEXCEPT ZMQPP_EXPLICITLY_DELETED;
};

}

#endif

#endif /* ZMQPP_AUTH_HPP_ */
