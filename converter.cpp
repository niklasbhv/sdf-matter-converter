//
// Created by Niklas on 26.02.2024.
//
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

using json = nlohmann::json;

int ConvertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    return 0;
}

int ConvertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    return 0;
}
