# Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

cmake_minimum_required(VERSION 3.8)
project(lexy VERSION 2022.12.1 LANGUAGES CXX)

set(LEXY_USER_CONFIG_HEADER "" CACHE FILEPATH "The user config header for lexy.")
option(LEXY_FORCE_CPP17     "Whether or not lexy should use C++17 even if compiler supports C++20." OFF)

add_subdirectory(src)

option(LEXY_ENABLE_INSTALL   "whether or not to enable the install rule" ON)
if(LEXY_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS lexy lexy_core lexy_file lexy_unicode lexy_ext _lexy_base lexy_dev
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE foonathan::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    configure_package_config_file(
        cmake/lexyConfig.cmake.in
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    # YYYY.MM.N1 is compatible with YYYY.MM.N2.
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY SameMinorVersion)

    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    install(DIRECTORY include/lexy include/lexy_ext
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
        PATTERN "*.hpp")
endif()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.18)
    option(LEXY_BUILD_BENCHMARKS "whether or not benchmarks should be built" OFF)
    option(LEXY_BUILD_EXAMPLES   "whether or not examples should be built" ON)
    option(LEXY_BUILD_TESTS      "whether or not tests should be built" ON)
    option(LEXY_BUILD_DOCS       "whether or not docs should be built" OFF)
    option(LEXY_BUILD_PACKAGE    "whether or not the package should be built" ON)

    if(LEXY_BUILD_PACKAGE)
        set(package_files include/ src/ cmake/ CMakeLists.txt LICENSE)
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip
            COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip --format=zip -- ${package_files}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${package_files})
        add_custom_target(lexy_package DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/lexy-src.zip)
    endif()

    if(LEXY_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()
    if(LEXY_BUILD_BENCHMARKS)
        add_subdirectory(benchmarks)
    endif()
    if(LEXY_BUILD_TESTS)
        set(DOCTEST_NO_INSTALL ON)
        enable_testing()
        add_subdirectory(tests)
    endif()
    if(LEXY_BUILD_DOCS)
        add_subdirectory(docs EXCLUDE_FROM_ALL)
    endif()
endif()
