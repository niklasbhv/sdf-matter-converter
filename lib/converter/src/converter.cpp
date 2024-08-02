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

int ConvertSdfToMatter(json& sdf_model_json, json& sdf_mapping_json,
                       std::optional<pugi::xml_document>& optional_device_xml,
                       std::list<pugi::xml_document>& cluster_xml_list)
{
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdf_mapping = sdf::ParseSdfMapping(sdf_mapping_json);
    std::optional<matter::Device> device;
    std::list<matter::Cluster> clusters;
    MapSdfToMatter(sdf_model, sdf_mapping, device, clusters);
    if (device.has_value()) {
        pugi::xml_document device_xml;
        SerializeDevice(device.value(), device_xml);
        optional_device_xml = std::move(device_xml);
    } else {
        optional_device_xml.reset();
    }

    for (const auto& cluster : clusters) {
        pugi::xml_document cluster_xml;
        SerializeCluster(cluster, cluster_xml);
        cluster_xml_list.push_back(std::move(cluster_xml));
    }

    return 0;
}

int ConvertMatterToSdf(const std::optional<pugi::xml_document>& device_xml,
                       const std::list<pugi::xml_document>& cluster_xml_list,
                       json& sdf_model_json, json& sdf_mapping_json)
{
    std::list<matter::Cluster> cluster_list;
    for (auto const& cluster_xml : cluster_xml_list) {
        matter::Cluster cluster =  matter::ParseCluster(cluster_xml.document_element());
        cluster_list.push_back(cluster);
    }

    sdf::SdfModel sdf_model;
    sdf::SdfMapping sdf_mapping;

    if (device_xml.has_value()) {
        matter::Device device = matter::ParseDevice(device_xml.value().document_element());
        MapMatterToSdf(device, cluster_list, sdf_model, sdf_mapping);
    } else {
        MapMatterToSdf(std::nullopt, cluster_list, sdf_model, sdf_mapping);
    }

    sdf_model_json = sdf::SerializeSdfModel(sdf_model);
    sdf_mapping_json = sdf::SerializeSdfMapping(sdf_mapping);

    return 0;
}
