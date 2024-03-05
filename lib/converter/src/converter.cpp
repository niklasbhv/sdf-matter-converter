//
// Created by Niklas on 26.02.2024.
//
#include "matter.h"
#include "sdf.h"
//TODO: Currently temporary
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
    std::cout << path << std::endl;
    if (!result){
        std::cout << "Could not load XML file" << std::endl;
        return -1;
    }
    std::cout << xml_file.root().name() << std::endl;
    return 0;
}

//TODO: This only works if we can store multiple cluster definitions inside a single xml file
int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    //TODO: We can initialize this as a fixed size array by using the number of dynamic endpoints
    std::cout << device_xml.value() << std::endl;
    std::list<deviceType> devices;
    //! Iterate through all deviceType children
    for (pugi::xml_node device_type_node: device_xml.children("configurator")) {
        deviceType device;
        //mapDevice(device_type_node, device);
        devices.push_back(device);
        const std::basic_string<char> device_name = device.name;
        std::cout << device.name << std::endl;
    }

    //mapCluster(cluster_xml);
    return 0;
}

int convertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    deviceType matter_device = {};
    //parseInfoBlock(sdf_model, matter_device);
    //parseNamespaceBlock();
    //parseDefinitionBlock(sdf_model, matter_device);
    return 0;
}
