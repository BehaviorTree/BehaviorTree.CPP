include_directories( ../sample_nodes )

set(CMAKE_DEBUG_POSTFIX "")

function(CompileExample name)
    add_executable(${name}  ${name}.cpp )
    target_link_libraries(${name} ${BTCPP_LIBRARY}  bt_sample_nodes )
endfunction()


# The plugin libdummy_nodes.so can be linked statically or
# loaded dynamically at run-time.
add_executable(t01_first_tree_static  t01_build_your_first_tree.cpp )
target_compile_definitions(t01_first_tree_static PRIVATE "MANUAL_STATIC_LINKING")
target_link_libraries(t01_first_tree_static ${BTCPP_LIBRARY}   bt_sample_nodes )

add_executable(t01_first_tree_dynamic  t01_build_your_first_tree.cpp )
target_link_libraries(t01_first_tree_dynamic ${BTCPP_LIBRARY}  )

CompileExample("t02_basic_ports")
CompileExample("t03_generic_ports")
CompileExample("t04_reactive_sequence")
CompileExample("t05_crossdoor")
CompileExample("t06_subtree_port_remapping")
CompileExample("t07_load_multiple_xml")
CompileExample("t08_additional_node_args")
CompileExample("t09_scripting")
CompileExample("t10_observer")
CompileExample("t11_replace_rules")

if(BTCPP_GROOT_INTERFACE AND BTCPP_SQLITE_LOGGING)
CompileExample("t12_groot_howto")
CompileExample("generate_log")
endif()

CompileExample("ex01_wrap_legacy")
CompileExample("ex02_runtime_ports")
CompileExample("ex04_waypoints")
CompileExample("ex05_subtree_model")
CompileExample("ex06_access_by_ptr")

CompileExample("t13_plugin_executor")

############ Create plugin for tutorial 13 ##########
# library must be SHARED
add_library(t13_plugin_action SHARED t13_plugin_action.cpp )
# you must set the definition BT_PLUGIN_EXPORT
target_compile_definitions(t13_plugin_action PRIVATE  BT_PLUGIN_EXPORT )
# remove the "lib" prefix. Name of the file will be t13_plugin_action.so
set_target_properties(t13_plugin_action PROPERTIES PREFIX "")
# link dependencies as usual
target_link_libraries(t13_plugin_action ${BTCPP_LIBRARY} )
