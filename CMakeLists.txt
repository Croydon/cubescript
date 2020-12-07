cmake_minimum_required(VERSION 3.15)
project(cubescript VERSION 0.2.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRES ON)


option(BUILD_REPL_TOOL "Build Read-eval-print loop tool" ON)
option(REPL_USE_LINENOISE "Use linenoise for the REPL" OFF)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CUBESCRIPT_EXTRA_CXX_FLAGS "-Wextra" "-Wshadow" "-Wold-style-cast")
else()
    set(CUBESCRIPT_EXTRA_CXX_FLAGS "")
endif()

set(MESON_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libostd")
file(MAKE_DIRECTORY ${MESON_WORKING_DIRECTORY})
execute_process(COMMAND meson setup ../subprojects/libostd ${MESON_WORKING_DIRECTORY} --default-library=static)

execute_process(COMMAND meson compile
                        WORKING_DIRECTORY ${MESON_WORKING_DIRECTORY})

find_library(LIBOSTD_LIBRARY libostd.a PATHS "build/libostd/src" NO_DEFAULT_PATH)

add_subdirectory("src")
add_subdirectory("tools")
