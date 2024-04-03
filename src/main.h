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
#ifdef VALIDATE
#include "validator.h"
#endif

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
inline int loadJsonFile(const char* path, nlohmann::json& json_file)
{
    std::ifstream f(path);
    json_file = nlohmann::json::parse(f);
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
inline int saveJsonFile(const char* path, nlohmann::json& json_file)
{
    std::ofstream f(path);
    f << json_file;
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
inline int loadXmlFile(const char* path, pugi::xml_document& xml_file)
{
    pugi::xml_parse_result result = xml_file.load_file(path);
    if (!result){
        std::cerr << "Could not load XML file" << std::endl;
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
inline int saveXmlFile(const char* path, pugi::xml_document& xml_file)
{
    return xml_file.save_file(path);
}

#endif //SDF_MATTER_CONVERTER_MAIN_H