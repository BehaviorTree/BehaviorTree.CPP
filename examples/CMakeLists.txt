cmake_minimum_required(VERSION 3.5.1)

include_directories( ../sample_nodes )

set(CMAKE_DEBUG_POSTFIX "")

function(CompileExample name)
    add_executable(${name}  ${name}.cpp )
    target_link_libraries(${name} ${BEHAVIOR_TREE_LIBRARY}  bt_sample_nodes )
endfunction()

CompileExample("t01_build_your_first_tree")
CompileExample("t02_basic_ports")
CompileExample("t03_generic_ports")
CompileExample("t04_reactive_sequence")
CompileExample("t05_crossdoor")
CompileExample("t06_subtree_port_remapping")
CompileExample("t07_load_multiple_xml")
CompileExample("t08_additional_node_args")
CompileExample("t09_scripting")
CompileExample("t10_observer")

CompileExample("ex01_wrap_legacy")
CompileExample("ex02_runtime_ports")
CompileExample("ex03_ncurses_manual_selector")
CompileExample("ex04_waypoints")
