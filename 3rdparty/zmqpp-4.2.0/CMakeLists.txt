# ZMQPP Cmake build description.
# ==============================
#
# CMake should find the zmq libraries / headers automatically if they are
# installed system-wide. If CMake cannot find them not, or you would like to
# use custom built ones, set these variables:
#
#   - ZEROMQ_LIB_DIR to the directory where libzmq / libzmq-shared is located
#
#   - ZEROMQ_INCLUDE_DIR to the directory where zmq.h is located
#


cmake_minimum_required(VERSION 2.8.12)
enable_testing()

# prepare C++11
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

# show all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# Set compiler flags that don't work on Windows
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()


# Set a consistent MACOSX_RPATH default across all CMake versions.  When CMake
# 2.8.12 is required, change this default to 1.  When CMake 3.0.0 is required,
# remove this block (see CMP0042).
#
# TODO: verify correctness of this flag
if(NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH 0)
endif()

# If libzmq is build in the same cmake global project and we want to
# depends on it instead of searching for libzmq in the system, we set this to true
set( ZMQPP_LIBZMQ_CMAKE   false   CACHE BOOL "libzmq is build through cmake too" )


set( ZMQPP_BUILD_STATIC   true   CACHE BOOL "Build the ZMQPP static library" )
set( ZMQPP_BUILD_SHARED   true   CACHE BOOL "Build the ZMQPP dynamic library" )

set( ZMQPP_BUILD_EXAMPLES false   CACHE BOOL "Build the ZMQPP examples" )
set( ZMQPP_BUILD_CLIENT   false   CACHE BOOL "Build the ZMQPP client" )
set( ZMQPP_BUILD_TESTS    false   CACHE BOOL "Build the ZMQPP tests" )


# Since the current CMake build of ZMQ does not work for generating a dynamic libzmq,
# give a chance for users to update which ZMQ library to link to

# zmq-static is the name of the static target in libzmq's CMakeLists.txt
set( ZMQPP_LIBZMQ_NAME_STATIC  "zmq-static" CACHE STRING "The ZMQ library to link the static ZMQPP. (if built)" )
set( ZMQPP_LIBZMQ_NAME_SHARED  "zmq"        CACHE STRING "The ZMQ library to link the dynamic ZMQPP. (if built)" )

# Paths to set to look for zmq
set( ZEROMQ_LIB_DIR       ""      CACHE PATH "The library directory for libzmq" )
set( ZEROMQ_INCLUDE_DIR   ""      CACHE PATH "The include directory for ZMQ" )

# Build flags
set( IS_TRAVIS_CI_BUILD   true    CACHE bool "Defines TRAVIS_CI_BUILD - Should the tests avoid running cases where memory is scarce." )

# Find zmq.h and add its dir to the includes
find_path(ZEROMQ_INCLUDE zmq.h PATHS ${ZEROMQ_INCLUDE_DIR})
include_directories(${ZEROMQ_INCLUDE_DIR} ${ZEROMQ_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/src )

# Do not run some tests when building on travis-ci (this cause oom error and kill the test
# process)
# These tests seem to be:
#    - sending_large_messages_string
if (IS_TRAVIS_CI_BUILD)
  add_definitions( -DTRAVIS_CI_BUILD)
endif()

set( INSTALL_TARGET_LIST )

# The library to link with the examples and the tests.
# Because we may or may not build shared/static libs, this needs to
# be dynamic
set( LIB_TO_LINK_TO_EXAMPLES )


# libzmqpp
# --------

set( LIBZMQPP_SOURCES
  src/zmqpp/actor.cpp
  src/zmqpp/context.cpp
  src/zmqpp/curve.cpp
  src/zmqpp/frame.cpp
  src/zmqpp/loop.cpp
  src/zmqpp/message.cpp
  src/zmqpp/poller.cpp
  src/zmqpp/reactor.cpp
  src/zmqpp/signal.cpp
  src/zmqpp/socket.cpp
  src/zmqpp/z85.cpp
  src/zmqpp/zap_request.cpp
  src/zmqpp/auth.cpp
  src/zmqpp/zmqpp.cpp
  src/zmqpp/proxy.cpp
  src/zmqpp/proxy_steerable.cpp
  )

# Staticlib
if (ZMQPP_BUILD_STATIC)
  add_library( zmqpp-static STATIC ${LIBZMQPP_SOURCES})
  target_compile_definitions(zmqpp-static PUBLIC ZMQ_STATIC ZMQPP_STATIC_DEFINE)
  if (NOT ZMQPP_LIBZMQ_CMAKE)
    find_library(ZEROMQ_LIBRARY_STATIC ${ZMQPP_LIBZMQ_NAME_STATIC} PATHS ${ZEROMQ_LIB_DIR})
    if (NOT ZEROMQ_LIBRARY_STATIC)
      # If libzmq was not installed through CMake, the static binary is libzmq.a not libzmq-static.a
      find_library(ZEROMQ_LIBRARY_STATIC libzmq.a PATHS ${ZEROMQ_LIB_DIR})
    endif()
    target_link_libraries( zmqpp-static ${ZEROMQ_LIBRARY_STATIC})
  else()
    # libzmq-static is the name of the target from
    # libzmq's CMake
    target_link_libraries(zmqpp-static libzmq-static)
  endif()
  list( APPEND INSTALL_TARGET_LIST zmqpp-static)
  set( LIB_TO_LINK_TO_EXAMPLES zmqpp-static )
endif() # ZMQPP_BUILD_STATIC

# Shared lib
if (ZMQPP_BUILD_SHARED)
  add_library( zmqpp SHARED ${LIBZMQPP_SOURCES})
  if (NOT ZMQPP_LIBZMQ_CMAKE)
    find_library(ZEROMQ_LIBRARY_SHARED ${ZMQPP_LIBZMQ_NAME_SHARED} PATHS ${ZEROMQ_LIB_DIR})
    target_link_libraries( zmqpp ${ZEROMQ_LIBRARY_SHARED} )
  else()
    # libzmq is the name of the target from
    # libzmq's CMake
    target_link_libraries(zmqpp libzmq)
  endif()
  list( APPEND INSTALL_TARGET_LIST zmqpp)
  set( LIB_TO_LINK_TO_EXAMPLES zmqpp )
endif() # ZMQPP_BUILD_SHARED

# We need to link zmqpp to ws2_32 on windows for the implementation of winsock2.h
if(WIN32)
	target_link_libraries(zmqpp ws2_32)
endif() # WIN32

include(GenerateExportHeader)
generate_export_header(zmqpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Examples
# --------

if(ZMQPP_BUILD_EXAMPLES)
  find_package(Threads REQUIRED)

  set (ZMQPP_EXAMPLES
    simple_server
    simple_client
    grasslands
    strawhouse
    woodhouse
    ironhouse
    ironhouse2)

  foreach( ZMQPP_EXAMPLE ${ZMQPP_EXAMPLES} )
    add_executable( zmqpp-example-${ZMQPP_EXAMPLE}  examples/${ZMQPP_EXAMPLE}.cpp )
    target_link_libraries( zmqpp-example-${ZMQPP_EXAMPLE} ${LIB_TO_LINK_TO_EXAMPLES} ${CMAKE_THREAD_LIBS_INIT})
    list( APPEND INSTALL_TARGET_LIST zmqpp-example-${ZMQPP_EXAMPLE} )
  endforeach()
endif()



# Client
# ------

if( ZMQPP_BUILD_CLIENT )
  # Boost
  # -----
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(Boost REQUIRED COMPONENTS program_options )
  include_directories( ${Boost_INCLUDE_DIRS} )

  add_executable( zmqpp-client
    src/client/main.cpp
    src/client/options.cpp
    )
  target_link_libraries( zmqpp-client ${LIB_TO_LINK_TO_EXAMPLES} ${Boost_LIBRARIES} )
  list( APPEND INSTALL_TARGET_LIST zmqpp-client )
endif()

# Tests
# -----

if( ZMQPP_BUILD_TESTS )
  #
  # Boost
  # -----
  set(Boost_USE_STATIC_LIBS OFF) # only find static libs
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(Boost REQUIRED COMPONENTS thread system unit_test_framework)
  include_directories( ${Boost_INCLUDE_DIRS} )

  add_executable( zmqpp-test-runner
    src/tests/test_actor.cpp
    src/tests/test_context.cpp
    src/tests/test_inet.cpp
    src/tests/test_load.cpp
    src/tests/test_message.cpp
    src/tests/test_message_stream.cpp
    src/tests/test_poller.cpp
    src/tests/test_reactor.cpp
    src/tests/test_loop.cpp
    src/tests/test_sanity.cpp
    src/tests/test_socket.cpp
    src/tests/test_socket_options.cpp
    src/tests/test_z85.cpp
    src/tests/test_auth.cpp
    src/tests/test_proxy.cpp
    )
  target_link_libraries( zmqpp-test-runner  ${LIB_TO_LINK_TO_EXAMPLES} ${Boost_LIBRARIES})
  add_test( zmqpp-test zmqpp-test-runner --log-level=test-suite )
endif()


# Install
# -------
install(TARGETS ${INSTALL_TARGET_LIST}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY src/zmqpp DESTINATION include/
        FILES_MATCHING PATTERN "*.hpp")

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/zmqpp_export.h"
        DESTINATION "include")
