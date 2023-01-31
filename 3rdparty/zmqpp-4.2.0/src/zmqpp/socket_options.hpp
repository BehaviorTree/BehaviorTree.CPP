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
 * \date   23 Sep 2011
 * \author Ben Gray (\@benjamg)
 */

#ifndef ZMQPP_SOCKET_OPTIONS_HPP_
#define ZMQPP_SOCKET_OPTIONS_HPP_

namespace zmqpp
{

/** \todo Expand the information on the options to make it actually useful. */
/*!
 * \brief possible Socket options in zmq
 */

ZMQPP_COMPARABLE_ENUM socket_option {
	affinity                  = ZMQ_AFFINITY,          /*!< I/O thread affinity */
	identity                  = ZMQ_IDENTITY,          /*!< Socket identity */
	subscribe                 = ZMQ_SUBSCRIBE,         /*!< Add topic subscription - set only */
	unsubscribe               = ZMQ_UNSUBSCRIBE,       /*!< Remove topic subscription - set only */
	rate                      = ZMQ_RATE,              /*!< Multicast data rate */
	send_buffer_size          = ZMQ_SNDBUF,            /*!< Kernel transmission buffer size */
	receive_buffer_size       = ZMQ_RCVBUF,            /*!< Kernel receive buffer size */
	receive_more              = ZMQ_RCVMORE,           /*!< Can receive more parts - get only */
	file_descriptor           = ZMQ_FD,                /*!< Socket file descriptor - get only */
	events                    = ZMQ_EVENTS,            /*!< Socket event flags - get only */
	type                      = ZMQ_TYPE,              /*!< Socket type - get only */
	linger                    = ZMQ_LINGER,            /*!< Socket linger timeout */
	backlog                   = ZMQ_BACKLOG,           /*!< Maximum length of outstanding connections - get only */
	reconnect_interval        = ZMQ_RECONNECT_IVL,     /*!< Reconnection interval */
	reconnect_interval_max    = ZMQ_RECONNECT_IVL_MAX, /*!< Maximum reconnection interval */
	receive_timeout           = ZMQ_RCVTIMEO,          /*!< Maximum inbound block timeout */
	send_timeout              = ZMQ_SNDTIMEO,          /*!< Maximum outbound block timeout */

#if (ZMQ_VERSION_MAJOR == 2)
    // Note that this is inverse of the zmq names for version 2.x
	recovery_interval_seconds = ZMQ_RECOVERY_IVL,      /*!< Multicast recovery interval in seconds */
	recovery_interval         = ZMQ_RECOVERY_IVL_MSEC, /*!< Multicast recovery interval in milliseconds */
	high_water_mark           = ZMQ_HWM,               /*!< High-water mark for all messages */
	swap_size                 = ZMQ_SWAP,              /*!< Maximum socket swap size in bytes */
	multicast_loopback        = ZMQ_MCAST_LOOP,        /*!< Allow multicast packet loopback */
#else // version > 2
	recovery_interval         = ZMQ_RECOVERY_IVL,      /*!< Multicast recovery interval in milliseconds */
	max_messsage_size         = ZMQ_MAXMSGSIZE,        /*!< Maximum inbound message size */
	send_high_water_mark      = ZMQ_SNDHWM,            /*!< High-water mark for outbound messages */
	receive_high_water_mark   = ZMQ_RCVHWM,            /*!< High-water mark for inbound messages */
	multicast_hops            = ZMQ_MULTICAST_HOPS,    /*!< Maximum number of multicast hops */

 #if (ZMQ_VERSION_MAJOR > 3) || ((ZMQ_VERSION_MAJOR == 3) && (ZMQ_VERSION_MINOR >= 1))
	ipv4_only                 = ZMQ_IPV4ONLY,
 #endif

 #if (ZMQ_VERSION_MAJOR > 3) || ((ZMQ_VERSION_MAJOR == 3) && (ZMQ_VERSION_MINOR >= 2))
  #if ((ZMQ_VERSION_MAJOR == 3) && (ZMQ_VERSION_MINOR == 2))
	delay_attach_on_connect   = ZMQ_DELAY_ATTACH_ON_CONNECT, /*!< Delay buffer attachment until connect complete */
  #else
	//  ZMQ_DELAY_ATTACH_ON_CONNECT is renamed in ZeroMQ starting 3.3.x
	immediate                 = ZMQ_IMMEDIATE,               /*!< Block message sending until connect complete */
  #endif
	last_endpoint             = ZMQ_LAST_ENDPOINT,           /*!< Last bound endpoint - get only */
	router_mandatory          = ZMQ_ROUTER_MANDATORY,        /*!< Require routable messages - set only */
	xpub_verbose              = ZMQ_XPUB_VERBOSE,            /*!< Pass on existing subscriptions - set only */
	tcp_keepalive             = ZMQ_TCP_KEEPALIVE,           /*!< Enable TCP keepalives */
	tcp_keepalive_idle        = ZMQ_TCP_KEEPALIVE_IDLE,      /*!< TCP keepalive idle count (generally retry count) */
	tcp_keepalive_count       = ZMQ_TCP_KEEPALIVE_CNT,       /*!< TCP keepalive retry count */
	tcp_keepalive_interval    = ZMQ_TCP_KEEPALIVE_INTVL,     /*!< TCP keepalive interval */
	tcp_accept_filter         = ZMQ_TCP_ACCEPT_FILTER,       /*!< Filter inbound connections - set only */
 #endif

 #if (ZMQ_VERSION_MAJOR >= 4)
	ipv6                      = ZMQ_IPV6, /*!< IPv6 socket support status */
	mechanism                 = ZMQ_MECHANISM, /*!< Socket security mechanism - get only */
	plain_password            = ZMQ_PLAIN_PASSWORD, /*!< PLAIN password */
	plain_server              = ZMQ_PLAIN_SERVER, /*!< PLAIN server role */
	plain_username            = ZMQ_PLAIN_USERNAME, /*!< PLAIN username */
	zap_domain                = ZMQ_ZAP_DOMAIN, /*!< RFC 27 authentication domain */
	conflate                  = ZMQ_CONFLATE, /*!< Keep only last message - set only */
	curve_public_key          = ZMQ_CURVE_PUBLICKEY, /*!< CURVE public key */
	curve_secret_key          = ZMQ_CURVE_SECRETKEY, /*!< CURVE secret key */
	curve_server_key          = ZMQ_CURVE_SERVERKEY, /*!< CURVE server key */
	curve_server              = ZMQ_CURVE_SERVER, /*!< CURVE server role - set only */
	probe_router              = ZMQ_PROBE_ROUTER, /*!< Bootstrap connections to ROUTER sockets - set only */
	request_correlate         = ZMQ_REQ_CORRELATE, /*!< Match replies with requests - set only */
	request_relaxed           = ZMQ_REQ_RELAXED, /*!< Relax strict alternation between request and reply - set only */
	router_raw                = ZMQ_ROUTER_RAW, /*!< Switch ROUTER socket to raw mode - set only */
 #endif

 #if (ZMQ_VERSION_MAJOR > 4) || ((ZMQ_VERSION_MAJOR == 4) && (ZMQ_VERSION_MINOR >= 1))
	handshake_interval        = ZMQ_HANDSHAKE_IVL, /*!< Maximum handshake interval */
	type_of_service           = ZMQ_TOS, /*!< Type-of-Service socket override status */
	connect_rid               = ZMQ_CONNECT_RID, /*!< Assign the next outbound connection id - set only */
	ipc_filter_gid            = ZMQ_IPC_FILTER_GID, /*!< Group ID filters to allow new IPC connections - set only */
	ipc_filter_pid            = ZMQ_IPC_FILTER_PID, /*!< Process ID filters to allow new IPC connections - set only */
	ipc_filter_uid            = ZMQ_IPC_FILTER_UID, /*!< User ID filters to allow new IPC connections - set only */
	router_handover           = ZMQ_ROUTER_HANDOVER, /*!< Handle duplicate client identities on ROUTER sockets - set only */
 #endif
 #if (ZMQ_VERSION_MAJOR > 4) || ((ZMQ_VERSION_MAJOR == 4) && (ZMQ_VERSION_MINOR >= 2))
	connect_timeout           = ZMQ_CONNECT_TIMEOUT, /*< Connect system call timeout */
	gssapi_plaintext          = ZMQ_GSSAPI_PLAINTEXT, /*< GSSAPI plaintext (disabled) state */
	gssapi_principal          = ZMQ_GSSAPI_PRINCIPAL, /*< GSSAPI principal name */
	gssapi_server             = ZMQ_GSSAPI_SERVER, /*< GSSAPI server state */
	gssapi_service_principal   = ZMQ_GSSAPI_SERVICE_PRINCIPAL, /*< GSSAPI connected server principal name */
	heartbeat_interval        = ZMQ_HEARTBEAT_IVL, /*< Heartbeat interval for ZMPT - set only */
	heartbeat_timeout         = ZMQ_HEARTBEAT_TIMEOUT, /*< ZMPT heartbeat timeout - set only */
	heartbeat_ttl             = ZMQ_HEARTBEAT_TTL, /*< ZMPT heartbeat interval - set only */
	invert_matching           = ZMQ_INVERT_MATCHING, /*< ZMPT invert state for PUB/SUB message filters */
	multicast_max_tpdu        = ZMQ_MULTICAST_MAXTPDU, /*< Max size for multicast messages */
	socks_proxy               = ZMQ_SOCKS_PROXY, /*< SOCKS5 proxy address for routing tcp connections */
	stream_notify             = ZMQ_STREAM_NOTIFY, /*< Event state on connect/disconnection of peers */
	tpc_max_retransmit        = ZMQ_TCP_MAXRT, /*< Maximum retransmit timeout */
	use_fd                    = ZMQ_USE_FD, /*!< Use a pre-allocated file descriptor instead of allocating a new one */
	vmci_buffer_size          = ZMQ_VMCI_BUFFER_SIZE, /*< VMCI buffer size */
	vmci_buffer_min           = ZMQ_VMCI_BUFFER_MIN_SIZE, /*< VMCI minimum buffer size */
	vmci_buffer_max           = ZMQ_VMCI_BUFFER_MAX_SIZE, /*< VMCI maximum buffer size */
	vmci_connect_timeout      = ZMQ_VMCI_CONNECT_TIMEOUT, /*< VMCI connection attempt timeout */
	xpub_manual               = ZMQ_XPUB_MANUAL,
	xpub_nodrop               = ZMQ_XPUB_NODROP,
	xpub_verboser             = ZMQ_XPUB_VERBOSER, /*!< Pass on existing (un)subscriptions - set only */
	xpub_welcome_message      = ZMQ_XPUB_WELCOME_MSG,
  #endif
#endif // version > 2

#ifdef ZMQ_EXPERIMENTAL_LABELS
	receive_label             = ZMQ_RCVLABEL,          /*!< Received label part - get only */
#endif
};

}

#endif /* ZMQPP_SOCKET_OPTIONS_HPP_ */
