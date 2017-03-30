cmake_minimum_required(VERSION 2.8.3)
project(BTpp)

set(CMAKE_BUILD_TYPE Release)
add_definitions(-Wall -lglut -lGL -lgtest -std=c++11)

# Needed for using threads
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
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
find_package(GLUT REQUIRED)
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


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/ ${PROJECT_SOURCE_DIR}/lib/include/)

file(GLOB_RECURSE BTHeadLibrary include/*.h lib/include/*.h)

set(BTSrcLibrary
lib/src/action_node.cpp
lib/src/behavior_tree.cpp
lib/src/condition_node.cpp
lib/src/control_node.cpp
#src/decorator_retry_node.cpp
#src/decorator_negation_node.cpp
lib/src/draw.cpp
lib/src/exceptions.cpp
lib/src/leaf_node.cpp
lib/src/tick_engine.cpp
lib/src/parallel_node.cpp
lib/src/fallback_node.cpp
lib/src/sequence_node.cpp
lib/src/fallback_node_with_memory.cpp
lib/src/sequence_node_with_memory.cpp
lib/src/tree_node.cpp
lib/src/actions/action_test_node.cpp
lib/src/conditions/condition_test_node.cpp
#src/actions/ros_action.cpp
#src/conditions/ros_condition.cpp
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
    add_executable(btpp_gtest lib/gtest//gtest_tree.cpp ${BTSrcLibrary} ${BTHeadLibrary})
    target_link_libraries(btpp_gtest  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
endif(GTEST_FOUND)

######################################################
# COMPILING SAMPLE EXAMPLE
######################################################
add_executable(btpp_example src/main.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp_example  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

######################################################
# COMPILING LIBRARY
######################################################
add_library(btpp SHARED ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

######################################################
# INSTALLATION OF LIBRARY AND EXECUTABLE TO /usr/local
######################################################
install (TARGETS btpp_example DESTINATION bin)
install (TARGETS btpp DESTINATION lib)

