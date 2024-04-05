/**
 *  Copyright 2024 Niklas Meyer
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <iostream>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

int loadJsonFile(const char* path, nlohmann::json& json_file)
{
    try {
        std::ifstream f(path);
        json_file = nlohmann::json::parse(f);
    }
    catch (const std::exception& err) {
        std::cerr << "Failed to load JSON file: " << path << std::endl;
        std::cerr << err.what() << std::endl;
        return -1;
    }
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