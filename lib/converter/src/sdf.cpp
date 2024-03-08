//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int parseCommonQualities(const json& sdf_elem, sdfCommonType& commonQuality)
{
    if (sdf_elem.contains("description")){
        sdf_elem.at("description").get_to(commonQuality.description);
        std::cout << "Common Quality Description: " << commonQuality.description << std::endl;
    }
    if (sdf_elem.contains("label")){
        sdf_elem.at("label").get_to(commonQuality.label);
        std::cout << "Common Quality Label: " << commonQuality.label << std::endl;
    }
    if (sdf_elem.contains("$comment")){
        sdf_elem.at("$comment").get_to(commonQuality.$comment);
        std::cout << "Common Quality $Comment: " << commonQuality.$comment << std::endl;
    }
    if (sdf_elem.contains("sdfRef")){
        sdf_elem.at("sdfRef").get_to(commonQuality.sdfRef);
        std::cout << "Common Quality sdfRef: " << commonQuality.sdfRef << std::endl;
    }
    if (sdf_elem.contains("sdfRequired")){
        std::list<std::string> sdfRequired;
        sdf_elem.at("sdfRequired").get_to(sdfRequired);
        commonQuality.sdfRequired = sdfRequired;
        std::cout << "Common Quality sdfRequired Size: " << commonQuality.sdfRequired.size() << std::endl;
    }
    return 0;
}

//! Function prototype for parseSdfData
int parseSdfData(const json& sdfdata_json, sdfDataType& sdfData);

int parseSdfChoice(const json& sdfchoice_json, std::map<std::string, sdfDataType> sdfChoiceMap)
{
    for (const auto& choice : sdfchoice_json.items()){
        sdfDataType sdfData;
        parseSdfData(choice.value(), sdfData);
        sdfChoiceMap.insert({choice.key(), sdfData});
        std::cout << "sdfChoice Element: " << choice.key() << std::endl;
    }
    return 0;
}

int parseSdfData(const json& sdfdata_json, sdfDataType& sdfData)
{
    sdfCommonType commonQualities;
    parseCommonQualities(sdfdata_json, commonQualities);
    sdfData.commonQualities = commonQualities;
    if (sdfdata_json.contains("unit")){
        sdfdata_json.at("unit").get_to(sdfData.unit);
        std::cout << "sdfData Unit: " << sdfData.unit << std::endl;
    }
    if (sdfdata_json.contains("nullable")){
        sdfdata_json.at("nullable").get_to(sdfData.nullable);
        std::cout << "sdfData Nullable: " << sdfData.nullable << std::endl;
    }
    if (sdfdata_json.contains("contentFormat")){
        sdfdata_json.at("contentFormat").get_to(sdfData.contentFormat);
        std::cout << "sdfData contentFormat: " << sdfData.contentFormat << std::endl;
    }
    if (sdfdata_json.contains("sdfType")){
        sdfdata_json.at("sdfType").get_to(sdfData.sdfType);
        std::cout << "sdfData sdfType: " << sdfData.sdfType << std::endl;
    }
    if (sdfdata_json.contains("sdfChoice")){
        std::map<std::string, sdfDataType> sdfChoiceMap;
        parseSdfChoice(sdfdata_json.at("sdfChoice"), sdfChoiceMap);
        sdfData.sdfChoice = sdfChoiceMap;
    }
    if (sdfdata_json.contains("enum")){

    }
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

int parseSdfObject(const json& sdfobject_json, sdfObjectType& sdfObject)
{
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
        std::cout << "Property List Size: " << sdfPropertyMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdfobject_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
        std::cout << "Action List Size: " << sdfActionMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdfobject_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
        std::cout << "Event List Size: " << sdfEventMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfData")){
        std::map<std::string, sdfDataType> sdfDataMap;
        for (const auto& data : sdfobject_json.at("sdfData").items()) {
            sdfDataType sdfData;
            parseSdfData(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        std::cout << "Data List Size: " << sdfDataMap.size() << std::endl;
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
