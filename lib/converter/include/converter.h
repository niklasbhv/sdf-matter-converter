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
 */

#ifndef SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_CONVERTER_H_
#define SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_CONVERTER_H_

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <list>
#include <optional>
#include "matter_to_sdf.h"
#include "sdf_to_matter.h"

//! @brief Convert sdf to matter.
//!
//! This function converts a given sdf-model and sdf-mapping into the matter format.
//!
//! @param sdf_model The input sdf-model.
//! @param sdf_mapping The input sdf-mapping.
//! @param device_xml The output device definition.
//! @param cluster_xml The output cluster definition.
//! @return 0 on success, negative on failure.
int ConvertSdfToMatter(nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json,
                       pugi::xml_document& device_xml, std::list<pugi::xml_document>& cluster_xml_list);

//! @brief Convert sdf to matter.
//!
//! This function converts a given sdf-model and sdf-mapping into the matter format.
//!
//! @param device_xml The input sdf-model.
//! @param cluster_xml_list The input sdf-mapping.
//! @param sdf_model_json The output sdf-model.
//! @param sdf_mapping_json The output sdf-mapping.
//! @return 0 on success, negative on failure.
int ConvertMatterToSdf(const std::optional<pugi::xml_document>& device_xml,
                       const std::list<pugi::xml_document>& cluster_xml_list,
                       nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json);

#endif //SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_CONVERTER_H_
