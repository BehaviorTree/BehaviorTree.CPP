#---- Add the subdirectory cmake ----
set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

find_package(ZeroMQ)

if(BTCPP_MANUAL_SELECTOR)
    find_package(Curses REQUIRED)
endif()

find_package(ament_index_cpp REQUIRED)

set( BTCPP_EXTRA_LIBRARIES
    $<BUILD_INTERFACE:ament_index_cpp::ament_index_cpp>
    $<BUILD_INTERFACE:${ZeroMQ_LIBRARIES}>
    ${CURSES_LIBRARIES}
)

ament_export_dependencies(ament_index_cpp)

set( BTCPP_LIB_DESTINATION     lib )
set( BTCPP_INCLUDE_DESTINATION include )
set( BTCPP_BIN_DESTINATION     bin )

mark_as_advanced(
    BTCPP_EXTRA_LIBRARIES
    BTCPP_LIB_DESTINATION
    BTCPP_INCLUDE_DESTINATION
    BTCPP_BIN_DESTINATION )

macro(export_btcpp_package)
    ament_export_include_directories(include)
    ament_export_libraries(${BTCPP_LIBRARY})
    ament_package()
endmacro()
