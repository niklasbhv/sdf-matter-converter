# Set the project name
project(validator)

# Add a library with the above sources
add_library(${PROJECT_NAME} src/validator.cpp)

# add dependencies
include(../../cmake/CPM.cmake)

CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:pboettch/json-schema-validator#2.3.0")

target_include_directories( ${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
)

find_package(LibXml2 REQUIRED)

target_link_libraries(validator PUBLIC nlohmann_json::nlohmann_json nlohmann_json_schema_validator LibXml2::LibXml2)
