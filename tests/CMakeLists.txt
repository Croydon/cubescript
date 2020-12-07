cmake_minimum_required(VERSION 3.15)
project(tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRES ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif()

# --- Conan --- #
# Download Conan CMake wrapper
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake v0.15 from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/b9fdb9f674e89d990c7764117cd26ae2693ff022/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

# Include Conan CMake wrapper
include(${CMAKE_BINARY_DIR}/conan.cmake)

# Make sure we have a recent Conan version
conan_check(VERSION 1.29.2 REQUIRED)

# Execute `conan install ...` for the packages in conanfile.txt
conan_cmake_run(CONANFILE conanfile.txt
        BASIC_SETUP
        BUILD missing
        # UPDATE
        SETTINGS compiler.cppstd=20
        SETTINGS compiler.libcxx=libstdc++11
        )

find_package(GTest REQUIRED)
find_library(LIBOSTD_LIBRARY libostd.a PATHS "../build/libostd/src" REQUIRED NO_DEFAULT_PATH)

add_executable(${PROJECT_NAME} tests.cc)
target_include_directories(${PROJECT_NAME} BEFORE 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/cubescript
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../subprojects/libostd
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../tools
    )

target_link_libraries(${PROJECT_NAME} "${LIBOSTD_LIBRARY}" "${CMAKE_CURRENT_SOURCE_DIR}/../build/src/libcubescript.a" GTest::gtest_main)
