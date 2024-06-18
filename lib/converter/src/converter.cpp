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

int convertSdfToMatter(nlohmann::ordered_json& sdf_model, nlohmann::ordered_json& sdf_mapping, pugi::xml_document& cluster_xml)
{
    sdfModelType sdfModel;
    parseSdfModel(sdf_model, sdfModel);
    sdfMappingType sdfMapping;

    parseSdfMapping(sdf_mapping, sdfMapping);

    std::list<matter::clusterType> clusters;
    map_sdf_to_matter(sdfModel, sdfMapping, clusters);
    for (const auto& cluster : clusters) {
        serialize_cluster(cluster, cluster_xml);
    }

    return 0;
}

int convertSdfToMatter(nlohmann::ordered_json& sdf_model, nlohmann::ordered_json& sdf_mapping, pugi::xml_document& device_xml, pugi::xml_document& cluster_xml)
{
    sdfModelType sdfModel;
    parseSdfModel(sdf_model, sdfModel);
    sdfMappingType sdfMapping;

    parseSdfMapping(sdf_mapping, sdfMapping);

    matter::deviceType device;
    map_sdf_to_matter(sdfModel, sdfMapping, device);

    serialize_device(device, device_xml, cluster_xml);
    return 0;
}

int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml, nlohmann::ordered_json& sdf_model, nlohmann::ordered_json& sdf_mapping)
{
    matter::deviceType device;
    parse_device(device_xml.document_element(), cluster_xml.document_element(), device, false);

    sdfModelType sdfModel;
    sdfMappingType sdfMapping;
    map_matter_to_sdf(device, sdfModel, sdfMapping);

    serializeSdfModel(sdfModel, sdf_model);
    serializeSdfMapping(sdfMapping, sdf_mapping);
    return 0;
}

int convertMatterToSdf(const pugi::xml_document& cluster_xml, nlohmann::ordered_json& sdf_model, nlohmann::ordered_json& sdf_mapping)
{
    matter::clusterType cluster;
    parse_cluster(cluster_xml.document_element(), cluster);

    sdfModelType sdfModel;
    sdfMappingType sdfMapping;
    map_matter_to_sdf(cluster, sdfModel, sdfMapping);

    serializeSdfModel(sdfModel, sdf_model);
    serializeSdfMapping(sdfMapping, sdf_mapping);
    return 0;
}

int testJsonParseSerialize(nlohmann::ordered_json& sdf_model, nlohmann::ordered_json& sdf_mapping)
{
    sdfModelType sdfModel;
    parseSdfModel(sdf_model, sdfModel);
    sdfMappingType sdfMapping;
    parseSdfMapping(sdf_mapping, sdfMapping);


    sdf_model.clear();
    sdf_mapping.clear();
    serializeSdfModel(sdfModel, sdf_model);
    serializeSdfMapping(sdfMapping, sdf_mapping);
    return 0;
}

int testXmlParseSerialize(pugi::xml_document& device_xml, pugi::xml_document& cluster_xml)
{
    matter::deviceType device;
    parse_device(device_xml.document_element(), cluster_xml.document_element(), device, false);

    device_xml.reset();
    cluster_xml.reset();
    serialize_device(device, device_xml, cluster_xml);
    return 0;
}
