/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 * options.cpp
 *
 *  Created on: 6 Nov 2013
 *      Author: bgray
 */

#include <iostream>
#include <map>
#include <utility>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "options.hpp"

#ifndef BUILD_CLIENT_NAME
#define BUILD_CLIENT_NAME "zmqpp"
#endif


boost::program_options::options_description connection_options()
{
	boost::program_options::options_description options("Connection Options");
	options.add_options()
		( "annotate,a", "annotate output with direction" )
		( "bind,b", boost::program_options::value<std::vector<std::string>>(), "bind to specified endpoint" )
		( "connect,c", boost::program_options::value<std::vector<std::string>>(), "connect to specified endpoint" )
		( "detailed,d", "increased level of information displayed" )
		( "exit-when-no-input,x", "don't wait for (streamed) input; exit on zero message" )
		( "ignore-zeroes,z", "deprecated option, now the default state" )
		( "multipart,m", "deprecated option, now the default state" )
		( "singlepart,s", "treat each line as a new message" )
		( "verbose,v", "display output sent over socket to stderr" )
		;

	return options;
}

boost::program_options::options_description miscellaneous_options()
{
	boost::program_options::options_description options("Miscellaneous Options");
	options.add_options()
		( "version", "display version" )
		( "help",    "show this help page" )
		;

	return options;
}

typedef std::map<std::string, zmqpp::socket_type> socket_type_index;
socket_type_index socket_type_options()
{
	socket_type_index socket_types = {
			{ "push", zmqpp::socket_type::push },
			{ "pull", zmqpp::socket_type::pull },
			{ "pub",  zmqpp::socket_type::publish },   { "publish",   zmqpp::socket_type::publish },
			{ "sub",  zmqpp::socket_type::subscribe }, { "subscribe", zmqpp::socket_type::subscribe },
			{ "req",  zmqpp::socket_type::request },   { "request",   zmqpp::socket_type::request },
			{ "rep",  zmqpp::socket_type::reply },     { "reply",   zmqpp::socket_type::reply }
	};

	return socket_types;
}

client_options process_command_line(int argc, char const* argv[])
{
	boost::program_options::positional_options_description arguments;
	arguments.add("type", 1);
	arguments.add("connect", 1);

	boost::program_options::options_description all;
	all.add_options()
		( "type", "0mq socket type" )
		;
	all.add(miscellaneous_options());
	all.add(connection_options());

	boost::program_options::variables_map vm;
	client_options options;

	try
	{
		boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( all ).positional( arguments ).run(), vm );
	}
	catch(boost::program_options::too_many_positional_options_error& e)
	{
		std::cerr << "Too many arguments provided." << std::endl;
		options.show_usage = true;
	}
	catch(boost::program_options::unknown_option& e)
	{
		std::cerr << "Unknown option '" << e.get_option_name() << "'." << std::endl;
		options.show_usage = true;
	}
	catch(boost::program_options::error& e)
	{
		std::cerr << "command line parse error: " << e.what() << "'." << std::endl;
		options.show_usage = true;
	}

	socket_type_index socket_types = socket_type_options();
	if ( vm.count( "type" ) && ( socket_types.end() == socket_types.find( vm["type"].as<std::string>() ) ) )
	{
		std::cerr << "Unknown value '" << vm["type"].as<std::string>() << "' provided for 0mq socket type." << std::endl;
		options.show_usage = true;
	}

	if ( (0 == vm.count( "type" )) || vm.count( "help" ) || ( (0 == vm.count( "connect" )) && (0 == vm.count( "bind" )) ) )
	{
		options.show_usage = true;
	}

	options.show_version = ( vm.count( "version" ) > 0 );
	options.show_help = ( vm.count( "help" ) > 0 );

	if ( vm.count( "type" ) )    { options.type     = socket_types[ vm["type"].as<std::string>() ]; }
	if ( vm.count( "bind" ) )    { options.binds    = vm["bind"].as<std::vector<std::string>>(); }
	if ( vm.count( "connect" ) ) { options.connects = vm["connect"].as<std::vector<std::string>>(); }

	options.singlepart = ( vm.count( "singlepart" ) > 0 );

	options.annotate = ( vm.count( "annotate" ) > 0 );
	options.verbose = ( vm.count( "verbose" ) > 0 || vm.count( "detailed" ) > 0 );
	options.detailed = ( vm.count( "detailed" ) > 0 );
	options.exit_on_empty = ( vm.count( "exit-when-no-input" ) > 0 );

	return options;
}

std::ostream& show_usage(std::ostream& stream, std::string const& application_name)
{
	stream << "Usage: " BUILD_CLIENT_NAME " [options] SOCKETTYPE ENDPOINT" << std::endl;
	stream << "0mq command line client tool." << std::endl;
	stream << "SOCKETTYPE is one of the supported 0mq socket types." << std::endl;
	stream << "  pull, push, pub, sub, req, rep" << std::endl;
	stream << "ENDPOINT is any valid 0mq endpoint." << std::endl;

	return stream;
}

std::ostream& show_help(std::ostream& stream)
{
	stream << connection_options() << std::endl;
	stream << miscellaneous_options() << std::endl;

	return stream;
}
