cmake_minimum_required(VERSION 3.22.1) # version on Ubuntu Jammy
project(btcpp_tgt_link_libs LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake 1 CONFIG REQUIRED)
set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(behaviortree_cpp 4 CONFIG REQUIRED)
set(CMAKE_FIND_DEBUG_MODE FALSE)

add_executable(btcpp_sample main.cpp)

# Check it works with the same target that conan users use
target_link_libraries(btcpp_sample PRIVATE BT::behaviortree_cpp)
