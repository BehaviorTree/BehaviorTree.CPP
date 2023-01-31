/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 *  Created on: 16 Aug 2011
 *      Author: @benjamg
 */

#include <cstdlib>
#include <array>
#include <iostream>
#include <tuple>

#include <zmqpp/zmqpp.hpp>

#include "options.hpp"

#ifndef BUILD_CLIENT_NAME
#define BUILD_CLIENT_NAME "zmqpp"
#endif

int main(int argc, char const* argv[])
{
	client_options const options = process_command_line( argc, argv );

	if( options.show_version )
	{
		uint8_t major, minor, patch;
		zmqpp::zmq_version(major, minor, patch);

		std::cout << BUILD_CLIENT_NAME << " version " << zmqpp::version() << std::endl;
		std::cout << "  built against 0mq version " << static_cast<int>(major) << "." << static_cast<int>(minor) << "." << static_cast<int>(patch) << std::endl;

		return EXIT_FAILURE;
	}

	if( options.show_usage || options.show_help )
	{
		show_usage( std::cout, BUILD_CLIENT_NAME );
		if( options.show_help )
		{
			std::cout << std::endl;
			show_help( std::cout );
		}

		return EXIT_FAILURE;
	}

	bool can_send = false, can_recv = false, toggles = false;
	switch( options.type )
	{
	case zmqpp::socket_type::push:      can_send = true; break;
	case zmqpp::socket_type::pull:      can_recv = true; break;
	case zmqpp::socket_type::publish:   can_send = true; break;
	case zmqpp::socket_type::subscribe: can_recv = true; break;
	case zmqpp::socket_type::request:   can_send = true; toggles = true; break;
	case zmqpp::socket_type::reply:     can_recv = true; toggles = true; break;
	default:
		std::cerr << "Unsupported socket type" << std::endl;
		return EXIT_FAILURE;
	}

	int standardin = -1;
	// If we can send / toggle then we need stdin
	if( can_send || toggles )
	{
		if( options.verbose ) { std::cerr << "Connecting to stdin" << std::endl; }
		standardin = fileno(stdin);
		if ( standardin < 0 ) // really?
		{
			std::cerr << "Unable to get standard input, this might be an OS thing, sorry." << std::endl;
			return EXIT_FAILURE;
		}
	}

	zmqpp::context context;
	zmqpp::socket socket( context, options.type );

	// TODO: allow subscriptions on command line
	if( zmqpp::socket_type::subscribe == options.type ) { socket.subscribe( "" ); }

	if( !options.binds.empty() )
	{
		for(size_t i = 0; i < options.binds.size(); ++i)
		{
			if( options.verbose ) { std::cerr << "binding to " << options.binds[i] << std::endl; }
			try
			{
				socket.bind( options.binds[i] );
			}
			catch(zmqpp::zmq_internal_exception& e)
			{
				std::cerr << "failed to bind to endpoint " << options.binds[i] << ": " << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	if( !options.connects.empty() )
	{
		for(size_t i = 0; i < options.connects.size(); ++i)
		{
			if( options.verbose ) { std::cerr << "connecting to " << options.connects[i] << std::endl; }
			try
			{
				socket.connect( options.connects[i] );
			}
			catch(zmqpp::zmq_internal_exception& e)
			{
				std::cerr << "failed to bind to endpoint " << options.connects[i] << ": " << e.what() << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	zmqpp::poller poller;
	poller.add(socket);
	if( standardin >= 0 ) { poller.add( standardin ); }

	if( options.verbose && ( can_send || toggles ) )
	{
		std::cerr << "While sending packets is allowed data entered on standard in will be sent to the 0mq socket." << std::endl;
		if( options.singlepart )
		{
			std::cerr << "messages will be considered terminated by newline." << std::endl;
		}
		else
		{
			std::cerr << "Message parts will be considered terminated by newline." << std::endl;
			std::cerr << "Messages will be considered terminated by an empty part." << std::endl;
			std::cerr << "The empty part itself will not be included." << std::endl;
		}
		std::cerr << std::endl;

		if ( toggles && !can_send )
		{
			std::cerr << "Sending starts as disabled for this socket type." << std::endl;
			std::cerr << std::endl;
		}
	}

	if( options.detailed )
	{
		if( standardin >= 0 ) { if( options.annotate ) { std::cerr << "**: "; } std::cerr << "reading from stdin is enabled." << std::endl; }
		if( can_send ) { if( options.annotate ) { std::cerr << "**: "; } std::cerr << "sending via socket is enabled." << std::endl; }
		if( can_recv ) { if( options.annotate ) { std::cerr << "**: "; } std::cerr << "receiving via socket is enabled." << std::endl; }
		if( toggles ) { if( options.annotate ) { std::cerr << "**: "; } std::cerr << "socket will flip between send/recv." << std::endl; }
		if( options.annotate ) { std::cerr << "**: "; }	std::cerr << "Warning - Detailed logging is enabled." << std::endl;
	}

	zmqpp::message message;
	while(true)
	{
		poller.check_for(socket, (can_recv) ? zmqpp::poller::poll_in : zmqpp::poller::poll_none);
		if( standardin >= 0 )
		{
			poller.check_for(standardin, (can_send) ? zmqpp::poller::poll_in : zmqpp::poller::poll_none);
		}

		if( options.detailed )
		{
			if( options.annotate ) { std::cerr << "**: "; }
			std::cerr << "Polling for incomming message data." << std::endl;
		}

		if( poller.poll() )
		{
			if (poller.has_input(socket))
			{
				assert(can_recv);
				if( options.detailed )
				{
					if( options.annotate ) { std::cerr << "**: "; }
					std::cerr << "Message on socket." << std::endl;
				}

				do
				{
					std::string message;
					socket.receive(message);

					if( options.annotate ) { std::cout << "<<: "; }
					std::cout << message << std::endl;

				} while(socket.has_more_parts());

				if( options.annotate ) { std::cout << " --- " << std::endl; }
				else { std::cout << std::endl; }

				if (toggles)
				{
					can_recv = false;
					can_send = true;
					if( options.detailed )
					{
						if( options.annotate ) { std::cerr << "**: " << std::endl; }
						std::cerr << "Toggling to sending enabled" << std::endl;
					}
				}
			}

			if( (standardin >= 0) && poller.has_input( standardin ) )
			{
				assert(can_send);
				if( options.detailed )
				{
					if( options.annotate ) { std::cerr << "**: "; }
					std::cerr << "Data on stdin." << std::endl;
				}

				// TODO: handle cases where we actually read a mb of data from standard in and still don't have the terminator
				std::array<char, 1048576> buffer;
				size_t length = 0;
				char* result = fgets( buffer.data(), buffer.size(), stdin );

				if( !result )
				{
					if( options.annotate ) { std::cerr << "!!: "; }

					std::cerr << "Error in standard input" << std::endl;
					return EXIT_FAILURE;
				}

				assert(message.parts() == 0);
				while( result && (length = strlen( buffer.data() ) - 1) > 0 ) // trim newline from gets
				{
					buffer[length] = 0;
					message.add_raw( buffer.data(), static_cast<uint64_t>(length) );

					if( options.singlepart ) { break; }

					result = fgets(buffer.data(), buffer.size(), stdin);
				}

				if( message.parts() > 0 )
				{
					if( options.verbose )
					{
						for( size_t i = 0; i < message.parts(); ++i )
						{
							if( options.annotate ) { std::cout << ">>: "; }
							std::cout << message.get(i) << std::endl;
						}

						if( options.annotate ) { std::cout << " --- " << std::endl; }
						else { std::cout << std::endl; }
					}

					if( !socket.send( message, true ) )
					{
						if( options.detailed )
						{
							if( options.annotate ) { std::cerr << "**: "; }
							std::cerr << "Output blocking, waiting to send" << std::endl;
						}

						if( !socket.send( message ) )
						{
							if( options.annotate ) {	std::cerr << "!!: "; }

							std::cerr << "Send failed, socket would have blocked" << std::endl;

							zmqpp::message tmp;
							std::swap( tmp, message );
						}
					}

					if (toggles)
					{
						can_recv = true;
						can_send = false;
						if( options.detailed )
						{
							if( options.annotate ) { std::cerr << "**: " << std::endl; }
							std::cerr << "Toggling to receive enabled" << std::endl;
						}
					}
				}
			}
			else if( (standardin >= 0) && can_send && !can_recv )
			{
				if( options.detailed )
				{
					if( options.annotate ) { std::cerr << "**: "; }
					std::cerr << "No data on stdin, exiting reader." << std::endl;
				}
				break;
			}
		}
		else if( options.detailed )
		{
			if( options.annotate ) { std::cerr << "**: "; }
			std::cerr << "Poller returned with no data, possibly an interrupt." << std::endl;
		}
	}

	if( options.detailed )
	{
		if( options.annotate ) { std::cerr << "**: "; }
		std::cerr << "Exited reader, shutting down." << std::endl;
	}

	return EXIT_SUCCESS;
}
