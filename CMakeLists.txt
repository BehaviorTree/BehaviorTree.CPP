cmake_minimum_required(VERSION 2.8.3)
project(BTpp)

# Needed for using threads
# Needed for using threads
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
FIND_PACKAGE(Boost REQUIRED system thread)

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


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/)

file(GLOB_RECURSE BTHeadLibrary include/*.h)


set(BTSrcLibrary
src/ActionNode.cpp
src/BehaviorTree.cpp
src/ConditionNode.cpp
src/ControlNode.cpp
src/DecoratorRetryNode.cpp
src/DecoratorNegationNode.cpp
src/Draw.cpp
src/Exceptions.cpp
src/LeafNode.cpp
src/NodeSemaphore.cpp
src/ParallelNode.cpp
src/SelectorNode.cpp
src/SelectorStarNode.cpp
src/SequenceStarNode.cpp
src/SequenceNode.cpp
src/TreeNode.cpp
src/Actions/ActionTestNode.cpp
src/Conditions/ConditionTestNode.cpp
)
add_executable(main src/main.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(main  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${Boost_THREAD_LIBRARY} )

add_executable(test src/test.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(test  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${Boost_THREAD_LIBRARY} )


