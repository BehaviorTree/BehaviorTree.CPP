list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

if(BTCPP_GROOT_INTERFACE)
    find_package(ZeroMQ REQUIRED)
    list(APPEND BTCPP_EXTRA_LIBRARIES $<IF:$<TARGET_EXISTS:libzmq-static>, libzmq-static, libzmq-shared>)
    list(APPEND BTCPP_EXTRA_INCLUDE_DIRS ${ZeroMQ_INCLUDE_DIRS})
    message(STATUS "ZeroMQ_LIBRARIES: ${ZeroMQ_LIBRARIES}")
endif()

if(BTCPP_SQLITE_LOGGING)
    find_package(SQLite3 REQUIRED)
    list(APPEND BTCPP_EXTRA_LIBRARIES SQLite::SQLite3)
    message(STATUS "SQLite3_LIBRARIES: ${SQLite3_LIBRARIES}")
endif()

if (NOT BTCPP_VENDOR_3RDPARTY)
    find_package(lexy CONFIG REQUIRED)
    find_package(minicoro REQUIRED)
    find_package(minitrace REQUIRED)
    find_package(wildcards REQUIRED)
    find_package(tinyxml2 REQUIRED)

    list(APPEND BTCPP_EXTRA_LIBRARIES foonathan::lexy minicoro::minicoro minitrace::minitrace wildcards::wildcards tinyxml2::tinyxml2)
    list(APPEND BTCPP_EXTRA_INCLUDE_DIRS ${lexy_INCLUDE_DIRS} ${minicoro_INCLUDE_DIRS} ${minitrace_INCLUDE_DIRS} ${wildcards_INCLUDE_DIRS} ${tinyxml2_INCLUDE_DIRS})
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
