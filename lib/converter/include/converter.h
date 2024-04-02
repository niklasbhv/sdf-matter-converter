/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Functions to load, convert and save xml and json files.
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

/**
 * @brief Load a json file.
 *
 * This function loads the json file for a given path.
 *
 * @param path The path to the file.
 * @param json_file The resulting json object.
 * @return 0 on success, negative on failure,
 */
int loadJsonFile(const char* path, nlohmann::json& json_file);

/**
 * @brief Convert sdf to matter.
 *
 * This function converts a given sdf-model and sdf-mapping into the matter format.
 *
 * @param sdf_model The input sdf-model.
 * @param sdf_mapping The input sdf-mapping.
 * @return 0 on success, negative on failure.
 */
int convertSdfToMatter(const nlohmann::json& sdf_model, const nlohmann::json& sdf_mapping);

/**
 * @brief Save a json object into a json file.
 *
 * This function saves a json object into a new json file.
 *
 * @param path The path to the file.
 * @param json_file The input json file.
 * @return 0 on success, negative on failure.
 */
int saveJsonFile(const char* path, nlohmann::json& json_file);

/**
 * @brief Load a xml file.
 *
 * This function loads the xml file for a given path.
 *
 * @param path The path to the file.
 * @param xml_file The resulting xml file.
 * @return 0 on success, negative on failure.
 */
int loadXmlFile(const char* path, pugi::xml_document& xml_file);

/**
 * @brief Convert matter to sdf.
 *
 * This function converts a given device definition and cluster definition into the sdf format.
 *
 * @param device_xml The input device definition.
 * @param cluster_xml The input cluster definition.
 * @return 0 on success, negative on failure.
 */
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml);

/**
 * @brief Save a xml object into a xml file.
 *
 * The function saves a xml object into a xml file.
 *
 * @param path The path to the file.
 * @param xml_file The input xml file.
 * @return 0 on success, negative on failure.
 */
int saveXmlFile(const char* path, pugi::xml_document& xml_file);

#endif //CONVERTER_H
