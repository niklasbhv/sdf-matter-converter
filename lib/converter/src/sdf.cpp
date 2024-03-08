//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int parseSdfEvent(const json& sdf_model)
{
    return 0;
}

int parseSdfAction(const json& sdf_model)
{
    return 0;
}

int parseSdfProperty(const json& sdf_model)
{
    return 0;
}

int parseSdfObject(const json& sdfobject_json, sdfObjectType& sdfObject)
{
    std::cout << sdfobject_json.size() << std::endl;
    if (sdfobject_json.contains("sdfProperty")){

    }
    if (sdfobject_json.contains("sdfAction")){

    }
    if (sdfobject_json.contains("sdfEvent")){

    }
    if (sdfobject_json.contains("sdfData")){

    }
    return 0;
}

int parseSdfThing(const json& sdf_elem, sdfThingType& sdfThing)
{
    return 0;
}

int parseNamespaceBlock(const json& namespace_json, namespaceType& nsp)
{
    for (const auto& nsp_item : namespace_json.items()) {
        nsp.namespaces.insert({nsp_item.key(), nsp_item.value()});
    }
    return 0;
}

int parseInfoBlock(const json& info_json, infoBlockType& infoBlock)
{
    if (info_json.contains("title"))
        info_json.at("title").get_to(infoBlock.title);
    if (info_json.contains("description"))
        info_json.find("description").value().get_to(infoBlock.description);
    if (info_json.contains("version"))
        info_json.at("version").get_to(infoBlock.version);
    if (info_json.contains("modified"))
        info_json.at("modified").get_to(infoBlock.modified);
    if (info_json.contains("copyright"))
        info_json.at("copyright").get_to(infoBlock.copyright);
    if (info_json.contains("license"))
        info_json.at("license").get_to(infoBlock.license);
    if (info_json.contains("features"))
        info_json.at("features").get_to(infoBlock.features);
    if (info_json.contains("$comment"))
        info_json.at("$comment").get_to(infoBlock.$comment);
    return 0;
}

int parseSdfModel(const json& sdf_model, std::list<sdfModelType>& sdfModelList)
{
    sdfModelType sdfModel;
    if (sdf_model.contains("info")) {
        infoBlockType infoBlock;
        parseInfoBlock(sdf_model.at("info"), infoBlock);
        sdfModel.infoBlock = infoBlock;
    }

    if (sdf_model.contains("namespace")) {
        if (sdf_model.contains("defaultNamespace")) {
            namespaceType nsp;
            parseNamespaceBlock(sdf_model.at("namespace"), nsp);
            sdf_model.at("defaultNamespace").get_to(nsp.defaultNamespace);
            sdfModel.namespaceBlock = nsp;
        }
    }

    //! Does the SDF-Model contain a sdfThing?
    if (sdf_model.contains("sdfThing")){
        std::map<std::string, sdfThingType>sdfThingMap;
        for (const auto& sdf_elem : sdf_model.at("sdfThing").items()) {
            sdfThingType sdfThing;
            parseSdfThing(sdf_elem.value(), sdfThing);
            sdfThingMap.insert({sdf_elem.key(), sdfThing});
        }
        sdfModel.sdfThings = sdfThingMap;
        std::cout << "Thing List Size: " << sdfThingMap.size() << std::endl;
    }
    //! If not, does the SDF-Model contain a sdfObject?
    else if (sdf_model.contains("sdfObject")){
        std::map<std::string, sdfObjectType> sdfObjectMap;
        for (const auto& sdf_elem : sdf_model.at("sdfObject").items()) {
            sdfObjectType sdfObject;
            parseSdfObject(sdf_elem.value(), sdfObject);
            sdfObjectMap.insert({sdf_elem.key(), sdfObject});
        }
        sdfModel.sdfObjects = sdfObjectMap;
        std::cout << "Object List Size: " << sdfObjectMap.size() << std::endl;
    }
    else {
        return -1;
    }
    sdfModelList.push_back(sdfModel);
    return 0;
}

int parseSdfMapping(const json& sdf_mapping)
{
    return 0;
}
