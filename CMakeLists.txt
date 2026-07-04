cmake_minimum_required(VERSION 3.20)

project(ShortestPathQueueStudy
    VERSION 0.1.0
    DESCRIPTION "Study peak BFS queue size for LeetCode 1091"
    LANGUAGES CXX
)

set(APP_NAME ShortestPathQueueStudy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

find_package(Threads REQUIRED)

add_executable(${APP_NAME}
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/bfs.cpp"
    "${SRC_DIR}/grid.cpp"
    "${SRC_DIR}/reference_grids.cpp"
    "${SRC_DIR}/search.cpp"
    "${SRC_DIR}/cli.cpp"
    "${SRC_DIR}/runner.cpp"
)

target_include_directories(${APP_NAME}
    PRIVATE
        "${INCLUDE_DIR}"
)

target_compile_options(${APP_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(${APP_NAME}
    PRIVATE
        Threads::Threads
)

if (CMAKE_BUILD_TYPE)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
else()
    set(BUILD_TYPE_LOWER "debug")
endif()

set_target_properties(${APP_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}"
)
