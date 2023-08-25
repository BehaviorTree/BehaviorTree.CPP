# - Try to find ZMQ
# Once done this will define
#
#  ZeroMQ_FOUND - system has ZMQ
#  ZeroMQ_INCLUDE_DIRS - the ZMQ include directory
#  ZeroMQ_LIBRARIES - Link these to use ZMQ
#  ZeroMQ_DEFINITIONS - Compiler switches required for using ZMQ
#
#  Copyright (c) 2011 Lee Hambley <lee.hambley@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if(ZeroMQ_FOUND)
  set(ZeroMQ_FOUND ${ZeroMQ_FOUND})
  set(ZeroMQ_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIR})
  set(ZeroMQ_LIBRARIES ${ZeroMQ_LIBRARY})
else()



if (ZeroMQ_LIBRARIES AND ZeroMQ_INCLUDE_DIRS)
  # in cache already
  set(ZeroMQ_FOUND TRUE)
else (ZeroMQ_LIBRARIES AND ZeroMQ_INCLUDE_DIRS)

  find_path(ZeroMQ_INCLUDE_DIR
    NAMES
      zmq.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
  )

  find_library(ZeroMQ_LIBRARY
    NAMES
      zmq
      libzmq
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(ZeroMQ_INCLUDE_DIRS
    ${ZeroMQ_INCLUDE_DIR}
  )

  if (ZeroMQ_LIBRARY)
    set(ZeroMQ_LIBRARIES
        ${ZeroMQ_LIBRARIES}
        ${ZeroMQ_LIBRARY}
    )
  endif (ZeroMQ_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(ZeroMQ DEFAULT_MSG ZeroMQ_LIBRARIES ZeroMQ_INCLUDE_DIRS)

  # show the ZeroMQ_INCLUDE_DIRS and ZeroMQ_LIBRARIES variables only in the advanced view
  mark_as_advanced(ZeroMQ_INCLUDE_DIRS ZeroMQ_LIBRARIES)

endif (ZeroMQ_LIBRARIES AND ZeroMQ_INCLUDE_DIRS)
endif(ZeroMQ_FOUND)
