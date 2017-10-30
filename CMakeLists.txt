cmake_minimum_required(VERSION 2.8)
project(BTpp)

set(CMAKE_BUILD_TYPE Release)
add_definitions(-lX11 -Wall -lglut -lGL -lgtest -std=c++11  -pthread -lrt )
# Needed for using threads
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")



#########################################################
# FIND X11
#########################################################
find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})

#########################################################
# FIND GTest
#########################################################
find_package(GTest)
include_directories(${GTEST_INCLUDE_DIRS})
if(NOT GTEST_FOUND)
    message(WARINING " GTest not found!")
endif(NOT GTEST_FOUND)

#########################################################
# FIND GLUT
#########################################################
find_package(GLUT)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gtest/include/)

if(APPLE)
   include_directories(AFTER "/opt/X11/include")
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    if(CMAKE_GENERATOR STREQUAL Xcode)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")
    endif()
endif()


#########################################################
# Make relative paths absolute (needed later on)
#########################################################

foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()




file(GLOB_RECURSE BTHeadLibrary include/*.h)

set(BTSrcLibrary
src/action_node.cpp
src/behavior_tree.cpp
src/condition_node.cpp
src/control_node.cpp
#src/draw.cpp
src/exceptions.cpp
src/leaf_node.cpp
src/tick_engine.cpp
src/parallel_node.cpp
src/fallback_node.cpp
src/sequence_node.cpp
src/fallback_node_with_memory.cpp
src/sequence_node_with_memory.cpp
src/tree_node.cpp

gtest/src/action_test_node.cpp
gtest/src/condition_test_node.cpp
)


######################################################
# SETTING BINARIES AND LIBRARY OUTPUT LOCATIONS
######################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sample)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

######################################################
# COMPILING GTEST
######################################################
if(GTEST_FOUND)
    add_executable(btpp_gtest gtest/gtest_tree.cpp ${BTSrcLibrary} ${BTHeadLibrary})
    target_link_libraries(btpp_gtest  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} ${X11_LIBRARIES})
endif(GTEST_FOUND)

######################################################
# COMPILING SAMPLE EXAMPLE
######################################################
add_executable(btpp_example src/example.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp_example  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_LIBRARIES})

######################################################
# COMPILING LIBRARY
######################################################
add_library(BTppLib STATIC ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(BTppLib ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_LIBRARIES})

######################################################
# INSTALLATION OF LIBRARY AND EXECUTABLE
######################################################



# Add all targets to the build-tree export set
export(TARGETS BTppLib
  FILE "${PROJECT_BINARY_DIR}/BTppTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE BTpp)


install(TARGETS BTppLib
  # IMPORTANT: Add the foo library to the "export-set"
  EXPORT BTppTargets
  ARCHIVE DESTINATION "${PROJECT_BINARY_DIR}/arc"
  RUNTIME DESTINATION "${PROJECT_BINARY_DIR}/bin" COMPONENT ${PROJECT_SOURCE_DIR}/bin
  LIBRARY DESTINATION "${PROJECT_BINARY_DIR}/lib" COMPONENT ${PROJECT_SOURCE_DIR}/shlib
  PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/BTpp"
    COMPONENT dev)


# Create the FooBarConfig.cmake and FooBarConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree


set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}")
configure_file(conf/BTppConfig.cmake.in
  "${PROJECT_BINARY_DIR}/BTppConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${BTpp_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(conf/BTppConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/BTppConfig.cmake" @ONLY)
# ... for both
configure_file(conf/BTppConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/BTppConfigVersion.cmake" @ONLY)

# Install the YARPBTCoreConfig.cmake and YARPBTCoreConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/BTppConfig.cmake"
  "${PROJECT_BINARY_DIR}/BTppConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT BTppTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)


