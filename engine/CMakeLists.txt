##################
# 'engine' library
##################

# define source files
include(../source_files/cmake_engine.txt)

# Define engine library or wasm executeable
if(SC_WASM)
    set(OPTS "")
    set(WASM_OPTS
        "-s TOTAL_MEMORY=256MB"
        "-s WASM_MEM_MAX=1024MB"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s MODULARIZE=1"
        "-s EXPORT_NAME=Simc"
        "-s DISABLE_EXCEPTION_CATCHING=0"
        "-s EXTRA_EXPORTED_RUNTIME_METHODS='[\"_free\", \"allocateUTF8\", \"UTF8ToString\"]'"
        "--js-library ${CMAKE_CURRENT_SOURCE_DIR}/wasm/glue.js"
        "--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/wasm/cache.js"
    )
    if(CMAKE_BUILD_TYPE MATCHES Release)
        list(APPEND OPTS -O3 -flto)
        list(APPEND WASM_OPTS -O3 "--llvm-lto 1")
    endif()
    add_executable(engine ${source_files} wasm/sc_wasm.cpp)
    target_compile_options(engine PRIVATE ${OPTS})
    target_compile_definitions(engine PUBLIC SC_WASM)
    target_link_libraries(engine PRIVATE ${WASM_OPTS})
else()
    add_library(engine ${source_files})
endif()
target_include_directories(engine PUBLIC . ./util)

# Make cmake selections visible to C++ code
if(SC_NO_THREADING)
    target_compile_definitions(engine PUBLIC SC_NO_THREADING)
endif()
if(SC_NO_NETWORKING)
    target_compile_definitions(engine PUBLIC SC_NO_NETWORKING)
endif()

# Detect pthreads
if(NOT SC_NO_THREADING)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(engine Threads::Threads)
endif()

# Networking libs
if(NOT SC_NO_NETWORKING)
    find_package(CURL REQUIRED)
    if (CURL_FOUND)
        target_link_libraries(engine ${CURL_LIBRARIES})
        target_include_directories(engine PUBLIC ${CURL_INCLUDE_DIRS})
    else(CURL_FOUND)
        message(FATAL_ERROR "libcurl not found")
    endif()
endif()

### Git Hash ###
# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(engine PUBLIC "SC_GIT_REV=\"${GIT_COMMIT_HASH}\"" "SC_GIT_BRANCH=\"${GIT_BRANCH}\"")
add_custom_target(update_git_info ALL DEPENDS util/git_info.cpp)
add_dependencies(engine update_git_info)
