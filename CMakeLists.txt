cmake_minimum_required(VERSION 3.27)
project(sdf_matter_converter)

set(CMAKE_CXX_STANDARD 17)

add_executable(sdf_matter_converter main.cpp
        converter.cpp
        validator.cpp)

# add dependencies
include(cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:zeux/pugixml@1.14")
CPMAddPackage("gh:p-ranav/argparse@3.0")

target_link_libraries(sdf_matter_converter nlohmann_json::nlohmann_json pugixml::pugixml argparse::argparse)