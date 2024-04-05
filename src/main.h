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

/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Functions to load and save xml and json files.
 */

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include "validator.h"

#ifndef SDF_MATTER_CONVERTER_MAIN_H
#define SDF_MATTER_CONVERTER_MAIN_H

/**
 * @brief Load a json file.
 *
 * This function loads the json file for a given path.
 *
 * @param path The path to the file.
 * @param json_file The resulting json object.
 * @return 0 on success, negative on failure,
 */
static inline int loadJsonFile(const char* path, nlohmann::json& json_file)
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
    return 0;
}

/**
 * @brief Save a json object into a json file.
 *
 * This function saves a json object into a new json file.
 *
 * @param path The path to the file.
 * @param json_file The input json file.
 * @return 0 on success, negative on failure.
 */
static inline int saveJsonFile(const char* path, nlohmann::json& json_file)
{
    try {
        std::ofstream f(path);
        f << json_file;
    }
    catch (const std::exception& err) {
        std::cerr << "Failed to save JSON file: " << path << std::endl;
        std::cerr << err.what() << std::endl;
        return -1;
    }
    return 0;
}

/**
 * @brief Load a xml file.
 *
 * This function loads the xml file for a given path.
 *
 * @param path The path to the file.
 * @param xml_file The resulting xml file.
 * @return 0 on success, negative on failure.
 */
static inline int loadXmlFile(const char* path, pugi::xml_document& xml_file)
{
    pugi::xml_parse_result result = xml_file.load_file(path);
    if (!result){
        std::cerr << "Failed to load XML file: " << path << std::endl;
        return -1;
    }
    return 0;
}

/**
 * @brief Save a xml object into a xml file.
 *
 * The function saves a xml object into a xml file.
 *
 * @param path The path to the file.
 * @param xml_file The input xml file.
 * @return 0 on success, negative on failure.
 */
static inline int saveXmlFile(const char* path, pugi::xml_document& xml_file)
{
    return xml_file.save_file(path);
}

#endif //SDF_MATTER_CONVERTER_MAIN_H
