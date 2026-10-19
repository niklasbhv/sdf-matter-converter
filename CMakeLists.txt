cmake_minimum_required(VERSION 3.22.1)
project(sdf_matter_converter)

set(CMAKE_CXX_STANDARD 17)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:zeux/pugixml@1.14")
CPMAddPackage("gh:p-ranav/argparse@3.0")

add_subdirectory(lib/converter)
add_subdirectory(lib/validator)

add_executable(sdf_matter_converter src/main.cpp src/main.h)

target_link_libraries(sdf_matter_converter validator converter nlohmann_json::nlohmann_json pugixml::pugixml argparse::argparse)

# Smoke tests for the command line tool
enable_testing()
add_subdirectory(tests)
