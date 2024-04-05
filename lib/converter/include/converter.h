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
 * Functions to convert between sdf and matter.
 *
 * @l
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
 * @param device_xml The output device definition.
 * @param cluster_xml The output cluster definition.
 * @return 0 on success, negative on failure.
 */
int convertSdfToMatter(const nlohmann::json& sdf_model, const nlohmann::json& sdf_mapping, pugi::xml_document& device_xml, pugi::xml_document& cluster_xml);

/**
 * @brief Convert matter to sdf.
 *
 * This function converts a given device definition and cluster definition into the sdf format.
 *
 * @param device_xml The input device definition.
 * @param cluster_xml The input cluster definition.
 * @param sdf_model The output sdf-model.
 * @param sdf_mapping The output sdf-mapping.
 * @return 0 on success, negative on failure.
 */
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml, nlohmann::json& sdf_model, nlohmann::json& sdf_mapping);

#endif //CONVERTER_H
