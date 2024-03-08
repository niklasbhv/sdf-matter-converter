//
// Created by Niklas on 26.02.2024.
//
#include "matter.h"
#include "sdf.h"
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

//TODO: This only works if we can store multiple cluster definitions inside a single xml file
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    std::list<deviceType> deviceList;
    parseDevices(device_xml, deviceList);
    std::list<clusterType> clusterList;
    parseClusters(cluster_xml, clusterList);
    std::cout << deviceList.size() << std::endl;
    std::cout << clusterList.size() << std::endl;
    return 0;
}

int convertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    std::list<sdfModelType> sdfModelList;
    parseSdfModel(sdf_model, sdfModelList);
    sdfMappingType sdfMapping;
    parseSdfMapping(sdf_mapping, sdfMapping);
    return 0;
}
