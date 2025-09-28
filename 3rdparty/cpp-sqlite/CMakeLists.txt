find_package(SQLite3 REQUIRED)

add_library(cpp-sqlite INTERFACE)

add_library(cpp-sqlite::cpp-sqlite ALIAS cpp-sqlite)

target_include_directories(cpp-sqlite
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cpp-sqlite
    INTERFACE
        SQLite::SQLite3
)
