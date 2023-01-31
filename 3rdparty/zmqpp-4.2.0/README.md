Introduction
============

Library / Bindings
------------------

This C++ binding for 0mq/zmq is a 'high-level' library that hides most of the
c-style interface core 0mq provides. It consists of a number of header and
source files all residing in the zmq directory, these files are provided under
the MPLv2 license (see LICENSE for details).

They can either be included directly into any 0mq using project or used as a
library. A really basic Makefile is provided for this purpose and will generate
both shared and static libraries.

There are a number of unit tests covering the code but in no way should the
tests be considered complete.

Command-Line Client
-------------------

There is also a command line client that can be used to test or even bridge
zmq connections. The client is built on top of the libzmqpp bindings.

Feature Requests
================

If there is any missing features from the current versions of ZeroMQ that you
wish to use please raise an issue against this project, or even a pull request.

Generally I've only added things as I need them but I'm always happy to improve
the feature set as people require.

Contributing
============

Contribution to this binding is welcome and it is suggested using pull requests
in github that will then be reviewed and merged or commented on. The full
contribution is outlined on the zmq site (http://zeromq.org/docs:contributing)

Please feel free to add yourself to the AUTHORS file in an alphanumerically
sorted way before you raise the pull request.

Documentation
=============

Most of the code is now commented with doxygen style tags, and a basic
configuration file to generate them is in the root directory.

To build the documentation with doxygen use

    doxygen

And the resulting html or latex docs will be in the docs/html or docs/latex
directories.


libzmqpp
========

There is a Makefile provided which will build, test and install the binding on
a GNU Linux system. I have not tested it on anything other than Ubuntu since
11.04 and Centos 5 and 6.

The install process will only install headers and the shared object to the
system. The archive will remain in the build directory.

The tests for the binding (make check) require the boost unittest framework to
have been installed however these do not need to be built or run to install
the library itself.

Requirements
------------

ZeroMQ 2.2.x or later. We recommend to use ZeroMQ >= 3.
C++11 compliant compiler. (g++ >= 4.7)

The command line client and the tests also require libboost.


Installation
------------

Installation can be done by the standard make && make install. If the boost
unittest framework is installed, check and installcheck can be run for sanity
checking. To use ZMQ4 security feature install libsodium and libzmq --with-libsodium
as shown below before ZMQPP.

    # Build, check, and install libsodium
    git clone git://github.com/jedisct1/libsodium.git
    cd libsodium
    ./autogen.sh 
    ./configure && make check 
    sudo make install 
    sudo ldconfig
    cd ../
    # Build, check, and install the latest version of ZeroMQ
    git clone git://github.com/zeromq/libzmq.git
    cd libzmq
    ./autogen.sh 
    ./configure --with-libsodium && make
    sudo make install
    sudo ldconfig
    cd ../
    # Now install ZMQPP
    git clone git://github.com/zeromq/zmqpp.git
    cd zmqpp
    make
    make check
    sudo make install
    make installcheck

The most commonly useful overrides are setting CXX, to change the compiler
used, and PREFIX to change install location. The CXX prefix should be used on
all targets as the compiler version is used in the build path. PREFIX is only
relevant for the install target.

Debugging
---------

The makefile defaults to a production ready build, however a debug version can
be build by passing CONFIG=debug to the make command. In debug mode there is
less optimisations and a number of sanity check assert statements. If you are
not using the installed library the sanity check effect is governed by the
defining of NDEBUG.


zmqpp
=====

The make file can also build and install a client tool called zmqpp. To build
this tool add the step;

    make client

Before the install stage. The install target will install the client to the
binaries directory if it has been built.

Usage
-----

The client is a command line tool that can be used to listen or send to 0mq
sockets. Its very basic so don't expect anything clever. zmqpp --help will list
details about the possible flags it can take;

    Usage: zmqpp [options] SOCKETTYPE ENDPOINT
    0mq command line client tool.
    SOCKETTYPE is one of the supported 0mq socket types.
      pub, pull, push, rep, req, sub
    ENDPOINT is any valid 0mq endpoint.

    Connection Options:
      -a [ --annotate ]            annotate output with direction
      -b [ --bind ] arg            bind to specified endpoint
      -c [ --connect ] arg         connect to specified endpoint
      -d [ --detailed ]            increased level of information displayed
      -x [ --exit-when-no-input ]  don't wait for (streamed) input; exit on zero 
                                   message
      -s [ --singlepart ]          treat each line as a new message
      -v [ --verbose ]             display output sent over socket to stderr

    Miscellaneous Options:
      --version             display version
      --help                show this help page

Multiple uses of -c or -b are allowed to connect or bind to multiple endpoints,
if neither is specified the connect is assumed for endpoint ENDPOINT.

For send capable sockets entering text on standard in and pressing return will
create a message part, with an empty part (double newline) marking the end of a
message. If singlepart is enabled then the message is sent after each newline
on the input stream.

The default flags will allow you to pipe data from one instance of zmqpp to
another and so bridge between zmq sockets.

Licensing
=========

Both the library and the associated command line client are released under the
MPLv2 license.

Please see LICENSE for full details.
