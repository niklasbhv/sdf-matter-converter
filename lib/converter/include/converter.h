/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Functions to convert between sdf and matter.
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

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
 * @brief Convert matter to sdf.
 *
 * This function converts a given device definition and cluster definition into the sdf format.
 *
 * @param device_xml The input device definition.
 * @param cluster_xml The input cluster definition.
 * @return 0 on success, negative on failure.
 */
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml);

#endif //CONVERTER_H
