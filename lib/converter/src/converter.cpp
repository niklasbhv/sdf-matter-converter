//
// Created by Niklas on 26.02.2024.
//
#include "matter.h"
#include "sdf.h"
#include "mapping.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <fstream>
#include <list>

using json = nlohmann::json;

int loadJsonFile(const char* path, json& json_file)
{
    std::ifstream f(path);
    //TODO: Check if this can fail (and how)
    json_file = json::parse(f);
    return 0;
}

int loadXmlFile(const char* path, pugi::xml_document& xml_file)
{
    pugi::xml_parse_result result = xml_file.load_file(path);
    if (!result){
        std::cerr << "Could not load XML file" << std::endl;
        return -1;
    }
    return 0;
}

int loadJsonFromCoap(){
    return 0;
}

int loadXmlFromCoap(){
    return 0;
}

int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    deviceType device;
    parseDevice(device_xml, device);
    std::list<clusterType> clusterList;
    parseClusters(cluster_xml, clusterList);
    std::cout << clusterList.size() << std::endl;
    map_matter_to_sdf(device, clusterList);
    return 0;
}

int convertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    sdfModelType sdfModel;
    parseSdfModel(sdf_model, sdfModel);
    sdfMappingType sdfMapping;
    parseSdfMapping(sdf_mapping, sdfMapping);
    map_sdf_to_matter(sdfModel, sdfMapping);
    return 0;
}
