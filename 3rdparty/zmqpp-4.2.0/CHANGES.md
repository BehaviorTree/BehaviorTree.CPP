Development
===========

Version 4.2.0

* New add_nocopy and add_ncopy_const functions for messages to allow better raw
  data handling.
* Actor execption handling propergation fixed.
* Support for 0mq 4.1 and 4.2 stable socket options and context options
* Removed libsodium dependency for test code as it is no longer required for
  0mq itself.
* Fixed NUL from permaturely terminating a string value socket option response.
* Support for removing parts of zmqpp::messages.
* Better support for msvc and avoiding windows dll issues.
* Non blocking functions default to bool type rather than integer flags.
* Cross platform endianness conversion.
* Code examples.

Version 4.1.2
=============

* Fix a compilation bug (#119)
* Improve documentation

Version 4.1.1
=============

Breaking
--------

* Removed message::add(pointer, size_t) as there were situations it conflicts
  with the new easier to use templated add. This has been replaced with a
  message::add_raw(pointer, size_t) method.

Other changes
-------------

* Added options to pop from/push to front of a message, this adds a new frame
  before the current ones.
* Added matching but redundant pop from/push to the end of the message.
* Support for remaining 0mq 3.2 and 4.0 socket options.
  * socket_option::conflate keep only one message in queues, ignores high water
    mark options. Only supports single part messages.
  * socket_option::curve_public_key set curve long term public key. This must
    be set on CURVE client sockets. You can provide the key as 32 binary bytes, 
    or as a 40-character string encoded in the Z85 encoding format.
  * socket_option::curve_secret_key set curve long term secret key. This must 
    be set on both CURVE client and server sockets. You can provide the key as  
    32 binary bytes, or as a 40-character string encoded in the Z85 encoding 
    format.
  * socket_option::curve_server_key set the long term server key. This must
    be set on CURVE client sockets. You can provide the key as 32 binary bytes, 
    or as a 40-character string encoded in the Z85 encoding format.
  * socket_option::curve_server defines if the socket will at as a server for 
    CURVE security. 
  * socket_option::ipv6 replacing the now deprecated ipv4only option, enables 
    support for ipv6.
  * socket_option::mechanism query the socket to find the current security 
    mechanism, if any.
  * socket_option::plain_password Sets the password for outgoing connections
    over TCP or IPC.
  * socket_option::plain_server defines whether the socket will act as server
    for PLAIN security.
  * socket_option::plain_username Sets the username for outgoing connections
    over TCP or IPC.
  * socket_option::probe_router Tell a router to automatically end an empty
    message when a new connection is made or accepted. You may set this on REQ,
    DEALER, or ROUTER sockets connected to a ROUTER socket.
  * socket_option::request_correlate Tell a REQ socket to prefix outgoing
    messages with an extra frame containing a request id.
  * socket_option::request_relaxed trigger reconnect on send instead of forcing
    a wait for previous reply.
  * socket_option::router_raw Don't apply 0mq framing to message, allowing talk
    to non-0mq sockets.
  * socket_option::zap_domain Domain value for ZAP (ZMQ RFC 27) authentication.
* Support for 4.0 context option.
  * context_option::ipv6 replacing the now deprecated ipv4only option, enables 
    support for ipv6.
* Basic wrapper of socket monitor, socket class now has a monitor function that
  setups the inproc monitor socket against it.
* Basic curve support. Allowing generation of keys via more c++ style interfaces
  as well as decoding and encoding z85 formatted data.

Version 3.2.0
=============

* Reworked client application, this is technically breaking but its only the
  the client so noone should care. Multipart support that works with linux
  pipes is now the default.
* Added a new -d detailed verbose option, the only reason it is not -vv is
  lack of boost option support. This provides diagnostic information.
* Added -s to support single part messages, incase anyone actually used that.

Version 3.1.0
=============

* Support for 3.2 socket disconnect and unbind.

Version 3.0.0
=============

Breaking
--------

* All constants are now lower_case rather the FULL_CAPITAL, this was due to
  defintiations conflicting with some of the values
* Message class methods that read data are now marked as const. This is only
  potentially breaking and probably will not harm most people.


Other changes
-------------

* Support for 3.2 style contexts and context options
  * context_option::io_threads to set the number of threads required. This
    must be set before sockets are created to have any effect. Default 1.
  * context_option::max_sockets to set the maximum number of sockets allowed
    by this context. Default 1024.
* Support for new 3.2 socket options
  * socket_option::delay_attach_on_connect to force connections to delay
    creating internal buffers until the connection to the remote endpoint.
  * socket_option::last_endpoint to get the last endpoint this socket was
    bound to.
  * socket_option::router_mandatory to enable routablity error checking on
    router sockets.
  * socket_option::xpub_verbose to force all subscriptions to bubble up the
    system rather than just new subscriptions.
  * socket_option::tcp_keepalive to enable tcp level keepalives, a special
    value of -1 is used for os default.
  * socket_option::tcp_keepalive_idle to force an overide of the keepalive
    count, often this is the retry count.
  * socket_option::tcp_keepalive_count to force an overide of the retry count.
  * socket_option::tcp_keepalive_interval to alter time between keepalives.
  * socket_option::tcp_accept_filter to add address based whitelist for
    incomming connections.

