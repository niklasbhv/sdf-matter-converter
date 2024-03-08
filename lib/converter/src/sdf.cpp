//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int parseSdfData(const json& sdf_model, sdfDataType& sdfData)
{
    return 0;
}

int parseSdfEvent(const json& sdf_model, sdfEventType& sdfEvent)
{
    return 0;
}

int parseSdfAction(const json& sdf_model, sdfActionType& sdfAction)
{
    return 0;
}

int parseSdfProperty(const json& sdf_model, sdfPropertyType& sdfProperty)
{
    return 0;
}

int parseCommonQualities(const json& sdf_elem, sdfCommonType& commonQuality)
{
    if (sdf_elem.contains("description")){
        sdf_elem.at("description").get_to(commonQuality.description);
    }
    if (sdf_elem.contains("label")){
        sdf_elem.at("label").get_to(commonQuality.label);
    }
    if (sdf_elem.contains("$comment")){
        sdf_elem.at("$common").get_to(commonQuality.$comment);
    }
    if (sdf_elem.contains("sdfRef")){

    }
    if (sdf_elem.contains("sdfRequired")){

    }
    return 0;
}

int parseSdfObject(const json& sdfobject_json, sdfObjectType& sdfObject)
{
    std::cout << sdfobject_json.size() << std::endl;
    sdfCommonType commonQualities;
    parseCommonQualities(sdfobject_json, commonQualities);
    sdfObject.commonQualities = commonQualities;
    if (sdfobject_json.contains("sdfProperty")){
        std::map<std::string, sdfPropertyType> sdfPropertyMap;
        for (const auto& property : sdfobject_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfPropertyMap.insert({property.key(), sdfProperty});
        }
    }
    if (sdfobject_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdfobject_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
    }
    if (sdfobject_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdfobject_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
    }
    if (sdfobject_json.contains("sdfData")){
        std::map<std::string, sdfDataType> sdfDataMap;
        for (const auto& data : sdfobject_json.at("sdfData").items()) {
            sdfDataType sdfData;
            parseSdfData(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
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
        for (const auto& thing : sdf_model.at("sdfThing").items()) {
            sdfThingType sdfThing;
            parseSdfThing(thing.value(), sdfThing);
            sdfThingMap.insert({thing.key(), sdfThing});
        }
        sdfModel.sdfThings = sdfThingMap;
        std::cout << "Thing List Size: " << sdfThingMap.size() << std::endl;
    }
    //! If not, does the SDF-Model contain a sdfObject?
    else if (sdf_model.contains("sdfObject")){
        std::map<std::string, sdfObjectType> sdfObjectMap;
        for (const auto& object : sdf_model.at("sdfObject").items()) {
            sdfObjectType sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdfObjectMap.insert({object.key(), sdfObject});
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
