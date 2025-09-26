add_library(flatbuffers INTERFACE)

add_library(flatbuffers::flatbuffers ALIAS flatbuffers)

target_include_directories(flatbuffers
    PUBLIC
        "."
)
