# Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
# SPDX-License-Identifier: BSL-1.0

add_executable(lexy_example_config)
set_target_properties(lexy_example_config PROPERTIES OUTPUT_NAME "config")
target_sources(lexy_example_config PRIVATE config.cpp)
target_link_libraries(lexy_example_config PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_calculator)
set_target_properties(lexy_example_calculator PROPERTIES OUTPUT_NAME "calculator")
target_sources(lexy_example_calculator PRIVATE calculator.cpp)
target_link_libraries(lexy_example_calculator PRIVATE foonathan::lexy::dev foonathan::lexy::unicode)

add_executable(lexy_example_email)
set_target_properties(lexy_example_email PROPERTIES OUTPUT_NAME "email")
target_sources(lexy_example_email PRIVATE email.cpp)
target_link_libraries(lexy_example_email PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_ip_address)
set_target_properties(lexy_example_ip_address PROPERTIES OUTPUT_NAME "ip_address")
target_sources(lexy_example_ip_address PRIVATE ip_address.cpp)
target_link_libraries(lexy_example_ip_address PRIVATE foonathan::lexy::dev)

add_executable(lexy_example_json)
set_target_properties(lexy_example_json PROPERTIES OUTPUT_NAME "json")
target_sources(lexy_example_json PRIVATE json.cpp)
target_link_libraries(lexy_example_json PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_protobuf)
set_target_properties(lexy_example_protobuf PROPERTIES OUTPUT_NAME "protobuf")
target_sources(lexy_example_protobuf PRIVATE protobuf.cpp)
target_link_libraries(lexy_example_protobuf PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_shell)
set_target_properties(lexy_example_shell PROPERTIES OUTPUT_NAME "shell")
target_sources(lexy_example_shell PRIVATE shell.cpp)
target_link_libraries(lexy_example_shell PRIVATE foonathan::lexy::dev)

add_executable(lexy_example_xml)
set_target_properties(lexy_example_xml PROPERTIES OUTPUT_NAME "xml")
target_sources(lexy_example_xml PRIVATE xml.cpp)
target_link_libraries(lexy_example_xml PRIVATE foonathan::lexy::dev foonathan::lexy::file)

add_executable(lexy_example_turing EXCLUDE_FROM_ALL)
set_target_properties(lexy_example_turing PROPERTIES OUTPUT_NAME "turing")
target_sources(lexy_example_turing PRIVATE turing.cpp)
target_link_libraries(lexy_example_turing PRIVATE foonathan::lexy::dev foonathan::lexy::file)

