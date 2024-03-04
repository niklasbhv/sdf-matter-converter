//
// Created by niklas on 28.02.24.
//

#ifndef CONVERTER_H
#define CONVERTER_H

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

int loadJsonFile(const char* path, nlohmann::json& json_file);
int convertSdfToMatter(const nlohmann::json& sdf_model, const nlohmann::json& sdf_mapping);
int loadXmlFile(const char* path);
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml);

#endif //CONVERTER_H
