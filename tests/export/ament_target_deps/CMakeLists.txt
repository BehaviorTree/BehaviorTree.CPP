cmake_minimum_required(VERSION 3.22.1) # version on Ubuntu Jammy
project(btcpp_ament_tgt_dep LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake 1 CONFIG REQUIRED)
find_package(behaviortree_cpp 4 REQUIRED)

add_executable(btcpp_sample main.cpp)

# This function doesn't yet support PRIVATE as an argument.
# See https://github.com/ament/ament_cmake/issues/158#issuecomment-475384147
# Instead, test preserving legacy usage with no linkage specifier.
ament_target_dependencies(btcpp_sample behaviortree_cpp)
