@mainpage

Introduction
------------

zmqpp is a "high-level" C++ binding for 0mq/zmq. The "high-level" term is used in
comparison to [cppzmq](https://github.com/zeromq/cppzmq) which is somewhat a raw wrapper
around the [libzmq](https://github.com/zeromq/libzmq) C interface.

The library is documented and has a nice test suite. This doesn't mean that the library is bug
free or that the test suite is 100% complete.

The development takes places in this [GitHub repository](http://github.com/zeromq/zmqpp).


Features
---------

Basic features:

+ zmqpp provides most feature from libzmq in a C++ style API.
+ It supports multiple version of [libzmq](https://github.com/zeromq/libzmq).

Being built on top of libzmq, and being a higher-level binding, zmqpp provides some
additional features:

+ [Reactor](@ref zmqpp::reactor) pattern.
+ [Actor](@ref zmqpp::actor) pattern.
+ Support for ZAP.


Examples
--------

zmqpp comes a few examples. These can be found [here](https://github.com/zeromq/zmqpp/tree/develop/examples).

Reading the documentation is a good way to start learning about zmqpp.
The most important classes that you will likely use are thoses:

+ [Context](@ref zmqpp::context).
+ [Socket](@ref zmqpp::socket).
+ [Message](@ref zmqpp::message).
+ And either a [Poller](@ref zmqpp::poller) or a [Reactor](@ref zmqpp::reactor).
