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

using json = nlohmann::ordered_json;

int ConvertSdfToMatter(json& sdf_model_json, json& sdf_mapping_json,
                       std::optional<pugi::xml_document>& device_xml, std::list<pugi::xml_document>& cluster_xml_list)
{
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdf_mapping = sdf::ParseSdfMapping(sdf_mapping_json);

    matter::Device device;
    std::list<matter::Cluster> clusters;
    //MapSdfToMatter(sdf_model, sdf_mapping, device, clusters);

    if (device_xml.has_value())
        device_xml = SerializeDevice(device);

    for (const auto& cluster : clusters) {
        pugi::xml_document cluster_xml = SerializeCluster(cluster);
        //cluster_xml_list.push_back(cluster_xml);
    }

    return 0;
}

int ConvertMatterToSdf(const std::optional<pugi::xml_document>& device_xml,
                       const std::list<pugi::xml_document>& cluster_xml_list,
                       json& sdf_model_json, json& sdf_mapping_json)
{
    std::optional<matter::Device> device;
    if (device_xml.has_value()) {
        device = matter::ParseDevice(device_xml.value().document_element(), false);
    }
    //std::list<matter::Cluster>
    for (auto const& cluster_xml : cluster_xml_list) {
        matter::Cluster cluster =  matter::ParseCluster(cluster_xml.document_element());

    }

    sdf::SdfModel sdf_model;
    sdf::SdfMapping sdf_mapping;
    //MapMatterToSdf(device, sdf_model, sdf_mapping);

    sdf_model_json = sdf::SerializeSdfModel(sdf_model);
    sdf_mapping_json = sdf::SerializeSdfMapping(sdf_mapping);

    return 0;
}

int TestJsonParseSerialize(nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json)
{
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdf_mapping = sdf::ParseSdfMapping(sdf_mapping_json);

    sdf_model_json.clear();
    sdf_mapping_json.clear();
    sdf_model_json = sdf::SerializeSdfModel(sdf_model);
    sdf_mapping_json = sdf::SerializeSdfMapping(sdf_mapping);

    return 0;
}

int TestXmlParseSerialize(pugi::xml_document& device_xml, pugi::xml_document& cluster_xml)
{
    matter::Device device;
    matter::Cluster cluster;
    device = matter::ParseDevice(device_xml.document_element(), false);
    cluster = matter::ParseCluster(cluster_xml.document_element());

    device_xml.reset();
    cluster_xml.reset();
    device_xml = matter::SerializeDevice(device);
    cluster_xml = matter::SerializeCluster(cluster);

    return 0;
}
