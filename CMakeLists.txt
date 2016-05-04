cmake_minimum_required(VERSION 2.8.3)
project(BTpp)

set(CMAKE_BUILD_TYPE Release)

# Needed for using threads
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#########################################################
# FIND BOOST
#########################################################
find_package(Boost REQUIRED system thread)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions(${Boost_DEFINITIONS})
if(NOT Boost_FOUND)
    message(ERROR " Boost not found!")
endif(NOT Boost_FOUND)
    
add_definitions(-Wall -lglut -lGL)

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
    lib/src/ActionNode.cpp
    lib/src/BehaviorTree.cpp
    lib/src/ConditionNode.cpp
    lib/src/ControlNode.cpp
    lib/src/DecoratorRetryNode.cpp
    lib/src/DecoratorNegationNode.cpp
    lib/src/Draw.cpp
    lib/src/Exceptions.cpp
    lib/src/LeafNode.cpp
    lib/src/NodeSemaphore.cpp
    lib/src/ParallelNode.cpp
    lib/src/SelectorNode.cpp
    lib/src/SelectorStarNode.cpp
    lib/src/SequenceStarNode.cpp
    lib/src/SequenceNode.cpp
    lib/src/TreeNode.cpp
    src/ActionNodeExample.cpp
    src/ConditionNodeExample.cpp
)

######################################################
# SETTING BINARIES AND LIBRARY OUTPUT LOCATIONS
######################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sample)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

######################################################
# COMPILING SAMPLE EXAMPLE
######################################################
add_executable(btpp_example src/main.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp_example  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${Boost_LIBRARIES})

######################################################
# COMPILING LIBRARY
######################################################
add_library(btpp SHARED ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${Boost_LIBRARIES})

######################################################
# INSTALLATION OF LIBRARY AND EXECUTABLE TO /usr/local
######################################################
install (TARGETS btpp_example DESTINATION bin)
install (TARGETS btpp DESTINATION lib)
#install (FILES "${PROJECT_BINARY_DIR???}/???.h"
#         DESTINATION include)
