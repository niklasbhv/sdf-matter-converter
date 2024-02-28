//
// Created by Niklas on 26.02.2024.
//
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <fstream>

using json = nlohmann::json;

int loadJsonFile(const char* path)
{
    //TODO: Path given is only to the folder
    std::ifstream f(path);
    //TODO: Check if this can fail (and how)
    json data = json::parse(f);
    return 0;
}

int loadXmlFile(const char* path)
{
    pugi::xml_document doc;
    //TODO: Path given is only to the folder
    pugi::xml_parse_result result = doc.load_file(path);
    if (!result)
        return -1;
    return 0;
}

int convertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    return 0;
}

int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    return 0;
}
