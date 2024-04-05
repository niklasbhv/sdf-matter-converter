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

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include "matter.h"
#include "sdf.h"
#include "mapping.h"

int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    deviceType device;
    parseDevice(device_xml.document_element(), cluster_xml.document_element(), device);
    map_matter_to_sdf(device);
    return 0;
}

int convertSdfToMatter(const nlohmann::json& sdf_model, const nlohmann::json& sdf_mapping)
{
    sdfModelType sdfModel;
    parseSdfModel(sdf_model, sdfModel);
    sdfMappingType sdfMapping;
    parseSdfMapping(sdf_mapping, sdfMapping);
    map_sdf_to_matter(sdfModel, sdfMapping);
    return 0;
}
