//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "mapping.h"

using json = nlohmann::json;

int parseCommonQualities(const json& sdf_elem, commonQualityType& commonQuality)
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
        sdf_elem.at("sdfRequired").get_to(commonQuality.sdfRequired);
        commonQuality.sdfRequired = sdfRequired;
        std::cout << "Common Quality sdfRequired Size: " << commonQuality.sdfRequired.size() << std::endl;
    }
    return 0;
}

//! Function prototype for parseDataQualities
int parseDataQualities(const json& data_qualities_json, dataQualityType& dataQuality);

int parseSdfChoice(const json& sdfchoice_json, std::map<std::string, dataQualityType> sdfChoiceMap)
{
    for (const auto& choice : sdfchoice_json.items()){
        dataQualityType dataQuality;
        parseDataQualities(choice.value(), dataQuality);
        sdfChoiceMap.insert({choice.key(), dataQuality});
        std::cout << "sdfChoice Element: " << choice.key() << std::endl;
    }
    return 0;
}

int parseDataQualities(const json& data_qualities_json, dataQualityType& dataQuality)
{
    parseCommonQualities(data_qualities_json, dataQuality);
    if (data_qualities_json.contains("unit")){
        data_qualities_json.at("unit").get_to(dataQuality.unit);
        std::cout << "dataQuality Unit: " << dataQuality.unit << std::endl;
    }
    if (data_qualities_json.contains("nullable")){
        data_qualities_json.at("nullable").get_to(dataQuality.nullable);
        std::cout << "dataQuality Nullable: " << dataQuality.nullable << std::endl;
    }
    if (data_qualities_json.contains("contentFormat")){
        data_qualities_json.at("contentFormat").get_to(dataQuality.contentFormat);
        std::cout << "dataQuality contentFormat: " << dataQuality.contentFormat << std::endl;
    }
    if (data_qualities_json.contains("sdfType")){
        data_qualities_json.at("sdfType").get_to(dataQuality.sdfType);
        std::cout << "dataQuality sdfType: " << dataQuality.sdfType << std::endl;
    }
    if (data_qualities_json.contains("sdfChoice")){
        std::map<std::string, dataQualityType> sdfChoiceMap;
        parseSdfChoice(data_qualities_json.at("sdfChoice"), sdfChoiceMap);
        dataQuality.sdfChoice = sdfChoiceMap;
    }
    if (data_qualities_json.contains("enum")){
        std::list<std::string> enumList;
        data_qualities_json.at("enum").get_to(enumList);
        dataQuality.enum_ = enumList;
    }
    return 0;
}

int parseSdfEvent(const json& sdfevent_json, sdfEventType& sdfEvent)
{
    parseCommonQualities(sdfevent_json, sdfEvent);
    if (sdfevent_json.contains("sdfOutputData")){
        dataQualityType sdfOutputData;
        parseDataQualities(sdfevent_json.at("sdfOutputData"), sdfOutputData);
        sdfEvent.sdfOutputData = sdfOutputData;
    }
    if (sdfevent_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdfevent_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfEvent.sdfData = sdfDataMap;
    }
    return 0;
}

int parseSdfAction(const json& sdfaction_json, sdfActionType& sdfAction)
{
    parseCommonQualities(sdfaction_json, sdfAction);
    if (sdfaction_json.contains("sdfInputData")){
        dataQualityType sdfInputData;
        parseDataQualities(sdfaction_json.at("sdfInputData"), sdfInputData);
        sdfAction.sdfInputData = sdfInputData;
    }
    if (sdfaction_json.contains("sdfOutputData")){
        dataQualityType sdfOutputData;
        parseDataQualities(sdfaction_json.at("sdfOutputData"), sdfOutputData);
        sdfAction.sdfOutputData = sdfOutputData;
    }
    if (sdfaction_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdfaction_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfAction.sdfData = sdfDataMap;
    }
    return 0;
}

int parseSdfProperty(const json& sdfproperty_json, sdfPropertyType& sdfProperty)
{
    parseDataQualities(sdfproperty_json, sdfProperty);
    if (sdfproperty_json.contains("readable")){
        sdfproperty_json.at("readable").get_to(sdfProperty.readable);
        std::cout << "sdfProperty readable: " << sdfProperty.readable << std::endl;
    }
    if (sdfproperty_json.contains("writable")){
        sdfproperty_json.at("writable").get_to(sdfProperty.writable);
        std::cout << "sdfProperty writable: " << sdfProperty.writable << std::endl;
    }
    if (sdfproperty_json.contains("observable")){
        sdfproperty_json.at("observable").get_to(sdfProperty.observable);
        std::cout << "sdfProperty observable: " << sdfProperty.observable << std::endl;
    }
    return 0;
}

int parseSdfObject(const json& sdfobject_json, sdfObjectType& sdfObject)
{
    parseCommonQualities(sdfobject_json, sdfObject);
    if (sdfobject_json.contains("sdfProperty")){
        std::map<std::string, sdfPropertyType> sdfPropertyMap;
        for (const auto& property : sdfobject_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfPropertyMap.insert({property.key(), sdfProperty});
        }
        sdfObject.sdfProperty = sdfPropertyMap;
        std::cout << "Property List Size: " << sdfPropertyMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdfobject_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
        sdfObject.sdfAction = sdfActionMap;
        std::cout << "Action List Size: " << sdfActionMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdfobject_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
        sdfObject.sdfEvent = sdfEventMap;
        std::cout << "Event List Size: " << sdfEventMap.size() << std::endl;
    }
    if (sdfobject_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdfobject_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfObject.sdfData = sdfDataMap;
        std::cout << "Data List Size: " << sdfDataMap.size() << std::endl;
    }
    if (sdfobject_json.contains("minItems")){
        sdfobject_json.at("minItems").get_to(sdfObject.minItems);
        std::cout << "sdfObject minItems: " << sdfObject.minItems << std::endl;
    }
    if (sdfobject_json.contains("maxItems")){
        sdfobject_json.at("maxItems").get_to(sdfObject.maxItems);
        std::cout << "sdfObject maxItems: " << sdfObject.maxItems << std::endl;
    }
    return 0;
}

int parseSdfThing(const json& sdfthing_json, sdfThingType& sdfThing)
{
    parseCommonQualities(sdfthing_json, sdfThing);
    if (sdfthing_json.contains("sdfObject")){
        std::map<std::string, sdfObjectType> sdfObjectMap;
        for (const auto& object : sdfthing_json.at("sdfProperty").items()) {
            sdfObjectType sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdfObjectMap.insert({object.key(), sdfObject});
        }
        sdfThing.sdfObject = sdfObjectMap;
        std::cout << "Property List Size: " << sdfObjectMap.size() << std::endl;
    }
    if (sdfthing_json.contains("sdfProperty")){
        std::map<std::string, sdfPropertyType> sdfPropertyMap;
        for (const auto& property : sdfthing_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfPropertyMap.insert({property.key(), sdfProperty});
        }
        sdfThing.sdfProperty = sdfPropertyMap;
        std::cout << "Property List Size: " << sdfPropertyMap.size() << std::endl;
    }
    if (sdfthing_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdfthing_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
        sdfThing.sdfAction = sdfActionMap;
        std::cout << "Action List Size: " << sdfActionMap.size() << std::endl;
    }
    if (sdfthing_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdfthing_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
        sdfThing.sdfEvent = sdfEventMap;
        std::cout << "Event List Size: " << sdfEventMap.size() << std::endl;
    }
    if (sdfthing_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdfthing_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfThing.sdfData = sdfDataMap;
        std::cout << "Data List Size: " << sdfDataMap.size() << std::endl;
    }
    if (sdfthing_json.contains("minItems")){
        sdfthing_json.at("minItems").get_to(sdfThing.minItems);
        std::cout << "sdfThing minItems: " << sdfThing.minItems << std::endl;
    }
    if (sdfthing_json.contains("maxItems")){
        sdfthing_json.at("maxItems").get_to(sdfThing.maxItems);
        std::cout << "sdfThing maxItems: " << sdfThing.minItems << std::endl;
    }
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

int parseSdfModel(const json& sdf_model, sdfModelType& sdfModel)
{
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
        sdfThingType sdfThing;
        parseSdfThing(sdf_model.at("sdfThing"), sdfThing);
    }
    //! If not, does the SDF-Model contain a sdfObject?
    else if (sdf_model.contains("sdfObject")) {
        sdfObjectType sdfObject;
        parseSdfObject(sdf_model.at("sdfObject"), sdfObject);
    }
    else {
        return -1;
    }
    return 0;
}

int parseSdfMapping(const json& sdf_mapping, sdfMappingType& sdfMapping)
{
    if (sdf_mapping.contains("info")) {
        infoBlockType infoBlock;
        parseInfoBlock(sdf_mapping.at("info"), infoBlock);
        sdfMapping.infoBlock = infoBlock;
    }
    if (sdf_mapping.contains("namespace")) {
        if (sdf_mapping.contains("defaultNamespace")) {
            namespaceType nsp;
            parseNamespaceBlock(sdf_mapping.at("namespace"), nsp);
            sdf_mapping.at("defaultNamespace").get_to(nsp.defaultNamespace);
            sdfMapping.namespaceBlock = nsp;
        }
    }
    if (sdf_mapping.contains("map")) {
        //TODO: Currently not sure how to implement different types and custom fields
        //! Iterate through all items in the map section
        for (auto& mapping : sdf_mapping.at("map").items()) {
            auto& link = mapping.key();
            pugi::xml_node current_node = mappingTree.root();
            int j = 0;
            //! Disassemble the JSON reference into its substrings and add them into a tree structure
            for (int i = 0; i < link.size(); i++){
                if (link[i] == '/') {
                    mappingTree.append_child(link.substr(j, i).c_str());
                    j = i;
                }
            }
            //! Iterate through all items available at this JSON reference
            for (auto& items : mapping.value().items()){
                //! Add a new attribute with its matching value
                mappingTree.append_attribute(items.key().c_str());
                //mappingTree.attribute(items.key().c_str()).set_value(items.value().get_to(char*));
            }

            std::cout << mappingTree.path() << std::endl;
        }
    }
    return 0;
}
