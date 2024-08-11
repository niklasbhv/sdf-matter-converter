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
#include "converter.h"

using json = nlohmann::ordered_json;

//! Function used to convert sdf to the matter data model
int ConvertSdfToMatter(json& sdf_model_json, json& sdf_mapping_json,
                       std::optional<pugi::xml_document>& optional_device_xml,
                       std::list<pugi::xml_document>& cluster_xml_list)
{
    // Parse the sdf-model and the sdf-mapping
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdf_mapping = sdf::ParseSdfMapping(sdf_mapping_json);

    std::optional<matter::Device> device;
    std::list<matter::Cluster> clusters;
    // Map sdf to the Matter data model
    MapSdfToMatter(sdf_model, sdf_mapping, device, clusters);
    // If the device type definition has a value, we can serialize it
    if (device.has_value()) {
        pugi::xml_document device_xml;
        SerializeDevice(device.value(), device_xml);
        optional_device_xml = std::move(device_xml);
    } else {
        optional_device_xml.reset();
    }

    // Serialize all clusters from the cluster list
    for (const auto& cluster : clusters) {
        pugi::xml_document cluster_xml;
        SerializeCluster(cluster, cluster_xml);
        cluster_xml_list.push_back(std::move(cluster_xml));
    }

    return 0;
}

//! Function used to convert the Matter data model to sdf
int ConvertMatterToSdf(const std::optional<pugi::xml_document>& device_xml,
                       const std::list<pugi::xml_document>& cluster_xml_list,
                       json& sdf_model_json, json& sdf_mapping_json)
{
    std::list<matter::Cluster> cluster_list;
    // Parse the list of given cluster definitions
    for (auto const& cluster_xml : cluster_xml_list) {
        matter::Cluster cluster =  matter::ParseCluster(cluster_xml.document_element());
        cluster_list.push_back(cluster);
    }

    sdf::SdfModel sdf_model;
    sdf::SdfMapping sdf_mapping;

    if (device_xml.has_value()) {
        // If a device type definition was provided, convert it with the cluster definitions to sdf
        matter::Device device = matter::ParseDevice(device_xml.value().document_element());
        MapMatterToSdf(device, cluster_list, sdf_model, sdf_mapping);
    } else {
        // Otherwise we just convert the list of clusters to sdf
        MapMatterToSdf(std::nullopt, cluster_list, sdf_model, sdf_mapping);
    }

    // Serialize the sdf-model as well as the sdf-mapping
    sdf_model_json = sdf::SerializeSdfModel(sdf_model);
    sdf_mapping_json = sdf::SerializeSdfMapping(sdf_mapping);

    return 0;
}
