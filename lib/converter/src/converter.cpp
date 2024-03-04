//
// Created by Niklas on 26.02.2024.
//
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <fstream>
#include <list>

using json = nlohmann::json;

//
// Definitions for Matter
//

struct eventType {

};

struct commandType {

};

struct attributeType {

};

struct clusterType {
    std::string name;
    //TODO: Can this be solved like this?
    std::string server;
    std::string client;
    std::string domain;
    std::string code;
    std::string define;
    std::string description;
    attributeType attributes[10];
    commandType commands[10];
    eventType events[10];
};

struct deviceType {
    std::string name;
    std::string domain;
    std::string typeName;
    std::string profileId;
    std::string deviceId;
    // TODO: Channels is currently missing
    std::list<clusterType> clusters;

};

//
// Definitions for SDF
//
// TODO: A lot of these qualities are n-ary, but currently in most cases only one object is possible

struct sdfCommonType {
    std::string description;
    std::string label;
    std::string $comment;
    // sdfRef
    // sdfRequired
};

struct sdfDataType {
    sdfCommonType commonQualities;
    std::string unit;
    bool nullable;
    std::string contentFormat;
    std::string sdfType;
    std::map<std::string, sdfDataType> sdfChoice;
    // std::string enum[];
};

struct sdfEventType {
    sdfCommonType commonQualities;
    std::map<std::string, sdfDataType> sdfOutputData;
    // sdfData
};

struct sdfActionType {
    sdfCommonType commonQualities;
    std::map<std::string, sdfDataType> sdfInputData;
    std::map<std::string, sdfDataType> sdfOutputData;
    // sdfData
};

// TODO:: Currently these default to false, check if there should be a default
struct sdfPropertyType {
    sdfDataType dataQualities;
    bool readable = false;
    bool writable = false;
    bool observable = false;
};

struct sdfObjectType {
    sdfCommonType commonQualities;
    std::list<sdfPropertyType> sdfProperty;
    std::list<sdfActionType> sdfAction;
    std::list<sdfEventType> sdfEvent;
    std::list<sdfDataType> sdfData;
    int minItems;
    int maxItems;
};

struct sdfThingType {
    sdfCommonType commonQualities;
    // sdfThing
    sdfObjectType sdfObject;
    sdfPropertyType sdfProperty;
    sdfActionType sdfAction;
    sdfEventType sdfEvent;
    sdfDataType sdfData;
    int minItems;
    int maxItems;

};

struct definitionType {

};

struct namespaceType {
    std::map<std::string, std::string> namespaces;
    std::string defaultNamespace;
};

struct infoBlockType{
    std::string title;
    std::string description;
    std::string version;
    std::string modified;
    std::string copyright;
    std::string license;
    std::string features;
    std::string $comment;
};

struct sdfModelType{
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    definitionType definitionBlock;
};

int loadJsonFile(const char* path, json& json_file)
{
    //TODO: Path given is only to the folder
    std::ifstream f(path);
    //TODO: Check if this can fail (and how)
    json_file = json::parse(f);
    return 0;
}

int loadXmlFile(const char* path, const pugi::xml_document& xml_file)
{
    pugi::xml_document doc;
    //TODO: Path given is only to the folder
    pugi::xml_parse_result result = doc.load_file(path);
    if (!result)
        return -1;
    return 0;
}

//
// Functions responsible for the Matter -> SDF conversion
//

int mapEvent()
{
    return 0;
}

int mapCommand()
{
    return 0;
}

int mapAttribute()
{
    return 0;
}

int mapCluster(const pugi::xml_document& cluster_xml)
{
    return 0;
}

int mapDevice(const pugi::xml_document& device_xml)
{
    deviceType device;
    //! Iterate through all deviceType children
    for (pugi::xml_node device_type_node: device_xml.children("configurator")){
        device.name = device_type_node.child("name").value();
        device.domain = device_type_node.child("domain").value();
        device.typeName = device_type_node.child("typeName").value();
        device.profileId = device_type_node.child("profileId").value();
        device.deviceId = device_type_node.child("deviceId").value();
        //! Iterate through all clusters children
        for (pugi::xml_node cluster_node: device_type_node.children("clusters"))
        {
            clusterType cluster;
            //TODO: Which of these do we need to keep? Consider round tripping
            cluster.name = cluster_node.child("include").attribute("cluster").value();
            cluster.client = cluster_node.child("include").attribute("client").value();
            cluster.server = cluster_node.child("include").attribute("server").value();
            //cluster_node.child("include").attribute("clientLocked").value();
            //cluster_node.child("include").attribute("serverLocked").value();
            device.clusters.push_back(cluster);
        }
    }
    return 0;
}

int convertMatterToSdf(const pugi::xml_document& device_xml, const pugi::xml_document& cluster_xml)
{
    mapDevice(device_xml);
    mapCluster(cluster_xml);
    return 0;
}

//
// Functions responsible for the SDF -> Matter conversion
//

clusterType mapSdfObject(const json& sdf_model)
{

    return clusterType {};
}

int mapSdfThing()
{
    return 0;
}

int parseDefinitionBlock(const json& sdf_model, deviceType& matter_device)
{
    //! Does the SDF-Model contain a sdfThing?
    if(sdf_model.contains("sdfThing")){

    }
    //! If not, does the SDF-Model contain a sdfObject?
    else if(sdf_model.contains("sdfObject")){
        //TODO: Does this break after we leave sdfObject?
        for (auto sdf_object_it = sdf_model.find("sdfObject"); sdf_object_it != sdf_model.end(); sdf_object_it++) {
            matter_device.clusters.push_back(mapSdfObject(*sdf_object_it));
        }
    }
    //! If no sdfThing and no sdfObject is present, there's something wrong
    //TODO: Do we eben have to check this?
    else {
        return -1;
    }
    return 0;
}

int parseNamespaceBlock()
{
    return 0;
}

int parseInfoBlock(const json& sdf_model, deviceType& matter_device)
{
    matter_device.name = "";
    matter_device.domain = "SDF";
    //matter_device.typeName = sdf_model.infoBlock.title;
    matter_device.profileId = "0";
    matter_device.deviceId = "0";
    return 0;
}

int convertSdfToMatter(const json& sdf_model, const json& sdf_mapping)
{
    deviceType matter_device = {};
    parseInfoBlock(sdf_model, matter_device);
    parseNamespaceBlock();
    parseDefinitionBlock(sdf_model, matter_device);
    return 0;
}