#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

int loadJsonFile(const char* path, nlohmann::json& json_file)
{
    std::ifstream f(path);
    json_file = nlohmann::json::parse(f);
    return 0;
}

int validateSdf(const char* path, const char* schema)
{
    //Load the json file as well as the schema
    nlohmann::json json_file;
    loadJsonFile(path, json_file);
    nlohmann::json json_schema;
    loadJsonFile(schema, json_schema);

    // Create a new validator and set its schema
    json_validator validator;
    try {
        validator.set_root_schema(json_schema);
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema failed: " << e.what() << "\n";
        return -1;
    }

    // Validate the json file against the schema
    try {
        auto defaultPatch = validator.validate(json_file);
        std::cout << "Validation succeeded\n";
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema failed: " << e.what() << "\n";
        return -1;
    }
    return 0;
}

int validateMatter(const char* path, const char* schema)
{
    return 0;
}