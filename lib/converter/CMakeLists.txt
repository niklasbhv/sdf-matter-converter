# Set the project name
project(converter)

# Add a library with the above sources
add_library(${PROJECT_NAME} src/converter.cpp
        src/mapping.cpp
        src/matter.cpp
        src/sdf.cpp
        include/mapping.h
        include/matter.h
        include/sdf.h)

# add dependencies
include(../../cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:zeux/pugixml@1.14")

target_include_directories( ${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(converter nlohmann_json::nlohmann_json pugixml::pugixml)
