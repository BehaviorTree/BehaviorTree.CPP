# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

find_program(HUGO_BINARY hugo)
if(NOT HUGO_BINARY)
    message(WARNING "hugo not found, won't be able to generate documentation")
endif()

add_custom_command(OUTPUT content/_index.adoc
        COMMAND ${CMAKE_COMMAND} -E copy ../README.adoc content/_index.adoc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../README.adoc)
add_custom_command(OUTPUT content/learn/changelog.adoc
        COMMAND ${CMAKE_COMMAND} -E echo "---"              >  content/learn/changelog.adoc
        COMMAND ${CMAKE_COMMAND} -E echo "title: Changelog" >> content/learn/changelog.adoc
        COMMAND ${CMAKE_COMMAND} -E echo "---"              >> content/learn/changelog.adoc
        COMMAND ${CMAKE_COMMAND} -E cat ../CHANGELOG.adoc   >> content/learn/changelog.adoc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../CHANGELOG.adoc)

set(resolver "${CMAKE_CURRENT_SOURCE_DIR}/../support/resolve-lexy-headers.py")
add_custom_command(OUTPUT assets/cpp/playground_headers.single.hpp
        COMMAND ${resolver} assets/cpp/playground_headers.hpp assets/cpp/playground_headers.single.hpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS foonathan::lexy)

add_custom_command(OUTPUT static/download
        COMMAND ${CMAKE_COMMAND} -E make_directory static/download
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(OUTPUT static/download/lexy-src.zip
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../lexy-src.zip static/download/lexy-src.zip
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS lexy_package)

set(generated_files content/_index.adoc content/learn/changelog.adoc assets/cpp/playground_headers.single.hpp static/download/lexy-src.zip)
add_custom_target(lexy_docs
        COMMAND ${HUGO_BINARY} --minify
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${generated_files})
add_custom_target(lexy_docs_serve
        COMMAND ${HUGO_BINARY} serve
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        USES_TERMINAL
        DEPENDS ${generated_files})

