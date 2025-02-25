# Fuzzing configuration
# Supports both local fuzzing and OSS-Fuzz integration

# Detect if we're running in OSS-Fuzz environment
if(DEFINED ENV{LIB_FUZZING_ENGINE})
  set(OSS_FUZZ ON)
  message(STATUS "OSS-Fuzz environment detected")
else()
  set(OSS_FUZZ OFF)
endif()

# Auto-detect AFL++ compiler if not in OSS-Fuzz mode
if(NOT OSS_FUZZ AND (CMAKE_C_COMPILER MATCHES ".*afl-.*" OR CMAKE_CXX_COMPILER MATCHES ".*afl-.*"))
  set(USE_AFLPLUSPLUS ON CACHE BOOL "Use AFL++ instead of libFuzzer" FORCE)
  message(STATUS "AFL++ compiler detected - automatically enabling AFL++ mode")
endif()

# When building for fuzzing, we want static library by default
set(BTCPP_SHARED_LIBS OFF CACHE BOOL "Build static library for fuzzing" FORCE)

# Only apply static linking settings if explicitly requested
if(FORCE_STATIC_LINKING)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(BUILD_SHARED_LIBS OFF)

  # Force static linking for dependencies
  if(BTCPP_GROOT_INTERFACE)
    set(ZeroMQ_USE_STATIC_LIBS ON)
    set(ZEROMQ_STATIC_LIBRARY ON)
  endif()

  if(BTCPP_SQLITE_LOGGING)
    set(SQLite3_USE_STATIC_LIBS ON)
  endif()
endif()

# Set up flags for local fuzzing (not used for OSS-Fuzz)
if(NOT OSS_FUZZ)
  list(APPEND BASE_FLAGS -O2)

  if(USE_AFLPLUSPLUS)
    set(SANITIZER_FLAGS
            -fsanitize=address,undefined
        )
  else()
    # For libFuzzer, use fuzzer-no-link for the library
    set(SANITIZER_FLAGS
            -fsanitize=address,undefined,fuzzer-no-link
        )
  endif()

  # Apply sanitizer flags to the base library
  list(APPEND BASE_FLAGS ${SANITIZER_FLAGS})

  add_compile_options(${BASE_FLAGS})
  add_link_options(${BASE_FLAGS})
endif()

# Disable certain features during fuzzing
set(BTCPP_EXAMPLES OFF CACHE BOOL "Disable examples during fuzzing" FORCE)
set(BTCPP_BUILD_TOOLS OFF CACHE BOOL "Disable tools during fuzzing" FORCE)
set(BTCPP_UNIT_TESTS OFF CACHE BOOL "Disable tests during fuzzing" FORCE)
set(BTCPP_SHARED_LIBS OFF CACHE BOOL "Build static library for fuzzing" FORCE)

# Function to apply fuzzing flags for local development builds
function(apply_local_fuzzing_flags target)
  target_compile_options(${target} PRIVATE
        ${BASE_FLAGS}
        ${SANITIZER_FLAGS}
    )

  if(FORCE_STATIC_LINKING)
    if(USE_AFLPLUSPLUS)
      target_link_options(${target} PRIVATE
                ${BASE_FLAGS}
                ${SANITIZER_FLAGS}
                -static-libstdc++
                -static-libgcc
                -fsanitize=fuzzer
            )
    else()
      target_link_options(${target} PRIVATE
                ${BASE_FLAGS}
                -fsanitize=fuzzer
                ${SANITIZER_FLAGS}
                -static-libstdc++
                -static-libgcc
            )
    endif()
  else()
    if(USE_AFLPLUSPLUS)
      target_link_options(${target} PRIVATE
                ${BASE_FLAGS}
                ${SANITIZER_FLAGS}
                -fsanitize=fuzzer
            )
    else()
      target_link_options(${target} PRIVATE
                ${BASE_FLAGS}
                -fsanitize=fuzzer
                ${SANITIZER_FLAGS}
            )
    endif()
  endif()
endfunction()

# Function to add fuzzing targets - compatible with both local and OSS-Fuzz builds
function(add_fuzzing_targets)
  set(FUZZERS bt_fuzzer script_fuzzer bb_fuzzer)

  foreach(fuzzer ${FUZZERS})
    add_executable(${fuzzer} fuzzing/${fuzzer}.cpp)

    if(OSS_FUZZ)
      # For OSS-Fuzz environment, we rely on environment variables
      # like $CC, $CXX, $CFLAGS, $CXXFLAGS, and $LIB_FUZZING_ENGINE
      target_link_libraries(${fuzzer} PRIVATE
                ${BTCPP_LIBRARY}
                ${BTCPP_EXTRA_LIBRARIES}
                $ENV{LIB_FUZZING_ENGINE}
            )
    else()
      # For local development, use our own flags
      apply_local_fuzzing_flags(${fuzzer})
      target_link_libraries(${fuzzer} PRIVATE
                ${BTCPP_LIBRARY}
                ${BTCPP_EXTRA_LIBRARIES}
            )
    endif()

    # Setup corpus directories (useful for both environments)
    set(CORPUS_DIR ${CMAKE_BINARY_DIR}/corpus/${fuzzer})
    file(MAKE_DIRECTORY ${CORPUS_DIR})
  endforeach()

  # Copy corpus files if they exist (useful for local testing)
  # OSS-Fuzz provides its own corpus handling
  if(NOT OSS_FUZZ)
    file(GLOB BT_CORPUS_FILES "${CMAKE_SOURCE_DIR}/fuzzing/corpus/bt_corpus/*")
    file(GLOB SCRIPT_CORPUS_FILES "${CMAKE_SOURCE_DIR}/fuzzing/corpus/script_corpus/*")
    file(GLOB BB_CORPUS_FILES "${CMAKE_SOURCE_DIR}/fuzzing/corpus/bb_corpus/*")

    if(BT_CORPUS_FILES)
      file(COPY ${BT_CORPUS_FILES} DESTINATION ${CMAKE_BINARY_DIR}/corpus/bt_fuzzer)
    endif()
    if(SCRIPT_CORPUS_FILES)
      file(COPY ${SCRIPT_CORPUS_FILES} DESTINATION ${CMAKE_BINARY_DIR}/corpus/script_fuzzer)
    endif()
    if(BB_CORPUS_FILES)
      file(COPY ${BB_CORPUS_FILES} DESTINATION ${CMAKE_BINARY_DIR}/corpus/bb_fuzzer)
    endif()
  endif()
endfunction()
