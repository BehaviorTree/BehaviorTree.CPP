list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

if(BTCPP_GROOT_INTERFACE)
    find_package(ZeroMQ REQUIRED)
    list(APPEND BTCPP_EXTRA_LIBRARIES ${ZeroMQ_LIBRARIES})
    list(APPEND BTCPP_EXTRA_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIRS})
    message(STATUS "ZeroMQ_LIBRARIES: ${ZeroMQ_LIBRARIES}")
endif()

if(BTCPP_SQLITE_LOGGING)
    find_package(SQLite3 REQUIRED)
    list(APPEND BTCPP_EXTRA_LIBRARIES ${SQLite3_LIBRARIES})
    message(STATUS "SQLite3_LIBRARIES: ${SQLite3_LIBRARIES}")
endif()


set( BTCPP_LIB_DESTINATION     lib )
set( BTCPP_INCLUDE_DESTINATION include )
set( BTCPP_BIN_DESTINATION     bin )

mark_as_advanced(
    BTCPP_EXTRA_LIBRARIES
    BTCPP_LIB_DESTINATION
    BTCPP_INCLUDE_DESTINATION
    BTCPP_BIN_DESTINATION )

macro(export_btcpp_package)

    install(EXPORT ${PROJECT_NAME}Targets
        FILE "${PROJECT_NAME}Targets.cmake"
        DESTINATION "${BTCPP_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
        NAMESPACE BT::
        )
    export(PACKAGE ${PROJECT_NAME})

    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${BTCPP_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
        )

    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${BTCPP_LIB_DESTINATION}/cmake/${PROJECT_NAME}"
        )
endmacro()
