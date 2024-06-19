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

int ConvertSdfToMatter(nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json, pugi::xml_document& cluster_xml)
{
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdf_mapping = sdf::ParseSdfMapping(sdf_mapping_json);

    std::list<matter::clusterType> clusters;
    map_sdf_to_matter(sdf_model, sdf_mapping, clusters);
    for (const auto& cluster : clusters) {
        serialize_cluster(cluster, cluster_xml);
    }

    return 0;
}

int ConvertSdfToMatter(nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json, pugi::xml_document& device_xml, pugi::xml_document& cluster_xml)
{
    sdf::SdfModel sdf_model = sdf::ParseSdfModel(sdf_model_json);
    sdf::SdfMapping sdfMapping = sdf::ParseSdfMapping(sdf_mapping_json);

    matter::deviceType device;
    map_sdf_to_matter(sdf_model, sdfMapping, device);

    serialize_device(device, device_xml, cluster_xml);

    return 0;
}

int ConvertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml, nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json)
{
    matter::deviceType device;
    parse_device(device_xml.document_element(), cluster_xml.document_element(), device, false);

    sdf::SdfModel sdf_model;
    sdf::SdfMapping sdf_mapping;
    map_matter_to_sdf(device, sdf_model, sdf_mapping);

    sdf_model_json = sdf::SerializeSdfModel(sdf_model);
    sdf_mapping_json = sdf::SerializeSdfMapping(sdf_mapping);

    return 0;
}

int ConvertMatterToSdf(const pugi::xml_document& cluster_xml, nlohmann::ordered_json& sdf_model_json, nlohmann::ordered_json& sdf_mapping_json)
{
    matter::clusterType cluster;
    parse_cluster(cluster_xml.document_element(), cluster);

    sdf::SdfModel sdf_model;
    sdf::SdfMapping sdf_mapping;
    map_matter_to_sdf(cluster, sdf_model, sdf_mapping);

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
    matter::deviceType device;
    parse_device(device_xml.document_element(), cluster_xml.document_element(), device, false);

    device_xml.reset();
    cluster_xml.reset();
    serialize_device(device, device_xml, cluster_xml);

    return 0;
}
