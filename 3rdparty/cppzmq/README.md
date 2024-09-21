[![CI](https://github.com/zeromq/cppzmq/actions/workflows/ci.yml/badge.svg)](https://github.com/zeromq/cppzmq/actions)
[![Coverage Status](https://coveralls.io/repos/github/zeromq/cppzmq/badge.svg?branch=master)](https://coveralls.io/github/zeromq/cppzmq?branch=master)
[![License](https://img.shields.io/github/license/zeromq/cppzmq.svg)](https://github.com/zeromq/cppzmq/blob/master/LICENSE)

Introduction & Design Goals
===========================

cppzmq is a C++ binding for libzmq. It has the following design goals:
 - cppzmq maps the libzmq C API to C++ concepts. In particular:
   - it is type-safe (the libzmq C API exposes various class-like concepts as void*)
   - it provides exception-based error handling (the libzmq C API provides errno-based error handling)
   - it provides RAII-style classes that automate resource management (the libzmq C API requires the user to take care to free resources explicitly)
 - cppzmq is a light-weight, header-only binding. You only need to include the header file zmq.hpp (and maybe zmq_addon.hpp) to use it.
 - zmq.hpp is meant to contain direct mappings of the abstractions provided by the libzmq C API, while zmq_addon.hpp provides additional higher-level abstractions.

There are other C++ bindings for ZeroMQ with different design goals. In particular, none of the following bindings are header-only:
 - [zmqpp](https://github.com/zeromq/zmqpp) is a high-level binding to libzmq.
 - [czmqpp](https://github.com/zeromq/czmqpp) is a binding based on the high-level czmq API.
 - [fbzmq](https://github.com/facebook/fbzmq) is a binding that integrates with Apache Thrift and provides higher-level abstractions in addition. It requires C++14.

Supported platforms
===================

 - Only a subset of the platforms that are supported by libzmq itself are supported. Some features already require a compiler supporting C++11. In the future, probably all features will require C++11. To build and run the tests, CMake and Catch are required.
 - Any libzmq 4.x version is expected to work. DRAFT features may only work for the most recent tested version. Currently explicitly tested libzmq versions are
   - 4.2.0 (without DRAFT API)
   - 4.3.4 (with and without DRAFT API)
 - Platforms with full support (i.e. CI executing build and tests)
   - Ubuntu 18.04 x64 (with gcc 4.8.5, 5.5.0, 7.5.0)
   - Ubuntu 20.04 x64 (with gcc 9.3.0, 10.3.0 and clang 12)
   - Visual Studio 2017 x64
   - Visual Studio 2019 x64
   - macOS 10.15 (with clang 12, without DRAFT API)
 - Additional platforms that are known to work:
   - We have no current reports on additional platforms that are known to work yet. Please add your platform here. If CI can be provided for them with a cloud-based CI service working with GitHub, you are invited to add CI, and make it possible to be included in the list above.
 - Additional platforms that probably work:
   - Any platform supported by libzmq that provides a sufficiently recent gcc (4.8.1 or newer) or clang (3.4.1 or newer)
   - Visual Studio 2012+ x86/x64

Examples
========
These examples require at least C++11.
```c++
#include <zmq.hpp>

int main()
{
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    sock.bind("inproc://test");
    sock.send(zmq::str_buffer("Hello, world"), zmq::send_flags::dontwait);
}
```
This a more complex example where we send and receive multi-part messages over TCP with a wildcard port.
```c++
#include <iostream>
#include <zmq_addon.hpp>

int main()
{
    zmq::context_t ctx;
    zmq::socket_t sock1(ctx, zmq::socket_type::push);
    zmq::socket_t sock2(ctx, zmq::socket_type::pull);
    sock1.bind("tcp://127.0.0.1:*");
    const std::string last_endpoint =
        sock1.get(zmq::sockopt::last_endpoint);
    std::cout << "Connecting to "
              << last_endpoint << std::endl;
    sock2.connect(last_endpoint);

    std::array<zmq::const_buffer, 2> send_msgs = {
        zmq::str_buffer("foo"),
        zmq::str_buffer("bar!")
    };
    if (!zmq::send_multipart(sock1, send_msgs))
        return 1;

    std::vector<zmq::message_t> recv_msgs;
    const auto ret = zmq::recv_multipart(
        sock2, std::back_inserter(recv_msgs));
    if (!ret)
        return 1;
    std::cout << "Got " << *ret
              << " messages" << std::endl;
    return 0;
}
```

See the `examples` directory for more examples. When the project is compiled with tests enabled, each example gets compiled to an executable.


API Overview
============

For an extensive overview of the `zmq.hpp` API in use, see this [Tour of CPPZMQ by @brettviren](https://brettviren.github.io/cppzmq-tour/index.html).

Bindings for libzmq in `zmq.hpp`:

Types:
* class `zmq::context_t`
* enum `zmq::ctxopt`
* class `zmq::socket_t`
* class `zmq::socket_ref`
* enum `zmq::socket_type`
* enum `zmq::sockopt`
* enum `zmq::send_flags`
* enum `zmq::recv_flags`
* class `zmq::message_t`
* class `zmq::const_buffer`
* class `zmq::mutable_buffer`
* struct `zmq::recv_buffer_size`
* alias `zmq::send_result_t`
* alias `zmq::recv_result_t`
* alias `zmq::recv_buffer_result_t`
* class `zmq::error_t`
* class `zmq::monitor_t`
* struct `zmq_event_t`,
* alias `zmq::free_fn`,
* alias `zmq::pollitem_t`,
* alias `zmq::fd_t`
* class `zmq::poller_t` DRAFT
* enum `zmq::event_flags` DRAFT
* enum `zmq::poller_event` DRAFT

Functions:
* `zmq::version`
* `zmq::poll`
* `zmq::proxy`
* `zmq::proxy_steerable`
* `zmq::buffer`
* `zmq::str_buffer`

Extra high-level types and functions `zmq_addon.hpp`:

Types:
* class `zmq::multipart_t`
* class `zmq::active_poller_t` DRAFT

Functions:
* `zmq::recv_multipart`
* `zmq::send_multipart`
* `zmq::send_multipart_n`
* `zmq::encode`
* `zmq::decode`

Compatibility Guidelines
========================

The users of cppzmq are expected to follow the guidelines below to ensure not to break when upgrading cppzmq to newer versions (non-exhaustive list):

* Do not depend on any macros defined in cppzmq unless explicitly declared public here.

The following macros may be used by consumers of cppzmq: `CPPZMQ_VERSION`, `CPPZMQ_VERSION_MAJOR`, `CPPZMQ_VERSION_MINOR`, `CPPZMQ_VERSION_PATCH`.

Contribution policy
===================

The contribution policy is at: http://rfc.zeromq.org/spec:22

Build instructions
==================

Build steps:

1. Build [libzmq](https://github.com/zeromq/libzmq) via cmake. This does an out of source build and installs the build files
   - download and unzip the lib, cd to directory
   - mkdir build
   - cd build
   - cmake ..
   - sudo make -j4 install

2. Build cppzmq via cmake. This does an out of source build and installs the build files
   - download and unzip the lib, cd to directory
   - mkdir build
   - cd build
   - cmake ..
   - sudo make -j4 install

3. Build cppzmq via [vcpkg](https://github.com/Microsoft/vcpkg/). This does an out of source build and installs the build files
   - git clone https://github.com/Microsoft/vcpkg.git
   - cd vcpkg
   - ./bootstrap-vcpkg.sh # bootstrap-vcpkg.bat for Powershell
   - ./vcpkg integrate install
   - ./vcpkg install cppzmq

Using this:

A cmake find package scripts is provided for you to easily include this library.
Add these lines in your CMakeLists.txt to include the headers and library files of
cpp zmq (which will also include libzmq for you).

```
#find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq)
target_link_libraries(*Your Project Name* cppzmq)
```
