# Set the project name
project(validator)

# Add a library with the above sources
add_library(${PROJECT_NAME} src/validator.cpp)

# add dependencies
include(../../cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:apache/xerces-c@3.2.5")
# CPMAddPackage("gh:pboettch/json-schema-validator@2.3.0")

target_include_directories( ${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(validator nlohmann_json::nlohmann_json xerces-c)
