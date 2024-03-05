cmake_minimum_required(VERSION 3.27)
project(sdf_matter_converter)

set(CMAKE_CXX_STANDARD 17)

add_executable(sdf_matter_converter src/main.cpp
        lib/converter/src/converter.cpp
        lib/validator/src/validator.cpp
        lib/converter/include/converter.h
        lib/validator/include/validator.h
        lib/converter/src/sdf.cpp
        lib/converter/include/sdf.h
        lib/converter/src/matter.cpp
        lib/converter/include/matter.h)

# add dependencies
include(cmake/CPM.cmake)

add_subdirectory(lib/converter)
add_subdirectory(lib/validator)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:zeux/pugixml@1.14")
CPMAddPackage("gh:p-ranav/argparse@3.0")

target_link_libraries(sdf_matter_converter validator converter nlohmann_json::nlohmann_json pugixml::pugixml argparse::argparse)