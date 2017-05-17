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


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gtest/include/)

#INCLUDE_DIRECTORIES(/usr/local/include/btpp/)

file(GLOB_RECURSE BTHeadLibrary include/*.h)

set(BTSrcLibrary
src/action_node.cpp
src/behavior_tree.cpp
src/condition_node.cpp
src/control_node.cpp
src/draw.cpp
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
    target_link_libraries(btpp_gtest  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
endif(GTEST_FOUND)

######################################################
# COMPILING SAMPLE EXAMPLE
######################################################
add_executable(btpp_example src/example.cpp ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp_example  ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

######################################################
# COMPILING LIBRARY
######################################################
add_library(btpp SHARED ${BTSrcLibrary} ${BTHeadLibrary})
target_link_libraries(btpp ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

######################################################
# INSTALLATION OF LIBRARY AND EXECUTABLE TO /usr/local
######################################################
install(TARGETS btpp
        EXPORT btpp-targets
        DESTINATION lib/btpp
)

install(FILES ${BTHeadLibrary} DESTINATION include/)


