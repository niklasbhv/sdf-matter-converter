/**
 *  Copyright 2024 Niklas Meyer
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "mapping.h"
#include "matter.h"
#include "sdf.h"

using json = nlohmann::ordered_json;

int parseCommonQualities(const json& common_qualities_json, commonQualityType& commonQuality)
{
    if (common_qualities_json.contains("description")){
        common_qualities_json.at("description").get_to(commonQuality.description);
        std::cout << "Common Quality Description: " << commonQuality.description << std::endl;
    }
    if (common_qualities_json.contains("label")){
        common_qualities_json.at("label").get_to(commonQuality.label);
        std::cout << "Common Quality Label: " << commonQuality.label << std::endl;
    }
    if (common_qualities_json.contains("$comment")){
        common_qualities_json.at("$comment").get_to(commonQuality.$comment);
        std::cout << "Common Quality $Comment: " << commonQuality.$comment << std::endl;
    }
    if (common_qualities_json.contains("sdfRef")){
        common_qualities_json.at("sdfRef").get_to(commonQuality.sdfRef);
        std::cout << "Common Quality sdfRef: " << commonQuality.sdfRef << std::endl;
    }
    if (common_qualities_json.contains("sdfRequired")){
        std::list<std::string> sdfRequired;
        common_qualities_json.at("sdfRequired").get_to(commonQuality.sdfRequired);
        commonQuality.sdfRequired = sdfRequired;
        std::cout << "Common Quality sdfRequired Size: " << commonQuality.sdfRequired.size() << std::endl;
    }
    return 0;
}

// Function prototype for parseDataQualities
int parseDataQualities(const json& data_qualities_json, dataQualityType& dataQuality);

int parseSdfChoice(const json& sdf_choice_json, std::map<std::string, dataQualityType> sdfChoiceMap)
{
    for (const auto& choice : sdf_choice_json.items()){
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

int parseSdfEvent(const json& sdf_event_json, sdfEventType& sdfEvent)
{
    parseCommonQualities(sdf_event_json, sdfEvent);
    if (sdf_event_json.contains("sdfOutputData")){
        dataQualityType sdfOutputData;
        parseDataQualities(sdf_event_json.at("sdfOutputData"), sdfOutputData);
        sdfEvent.sdfOutputData = sdfOutputData;
    }
    if (sdf_event_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdf_event_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfEvent.sdfData = sdfDataMap;
    }
    return 0;
}

int parseSdfAction(const json& sdf_action_json, sdfActionType& sdfAction)
{
    parseCommonQualities(sdf_action_json, sdfAction);
    if (sdf_action_json.contains("sdfInputData")){
        dataQualityType sdfInputData;
        parseDataQualities(sdf_action_json.at("sdfInputData"), sdfInputData);
        sdfAction.sdfInputData = sdfInputData;
    }
    if (sdf_action_json.contains("sdfOutputData")){
        dataQualityType sdfOutputData;
        parseDataQualities(sdf_action_json.at("sdfOutputData"), sdfOutputData);
        sdfAction.sdfOutputData = sdfOutputData;
    }
    if (sdf_action_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdf_action_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfAction.sdfData = sdfDataMap;
    }
    return 0;
}

int parseSdfProperty(const json& sdf_property_json, sdfPropertyType& sdfProperty)
{
    parseDataQualities(sdf_property_json, sdfProperty);
    if (sdf_property_json.contains("readable")){
        sdf_property_json.at("readable").get_to(sdfProperty.readable.value());
        std::cout << "sdfProperty readable: " << sdfProperty.readable.value() << std::endl;
    }
    if (sdf_property_json.contains("writable")){
        sdf_property_json.at("writable").get_to(sdfProperty.writable.value());
        std::cout << "sdfProperty writable: " << sdfProperty.writable.value() << std::endl;
    }
    if (sdf_property_json.contains("observable")){
        sdf_property_json.at("observable").get_to(sdfProperty.observable.value());
        std::cout << "sdfProperty observable: " << sdfProperty.observable.value() << std::endl;
    }
    return 0;
}

int parseSdfObject(const json& sdf_object_json, sdfObjectType& sdfObject)
{
    parseCommonQualities(sdf_object_json, sdfObject);
    if (sdf_object_json.contains("sdfProperty")){
        std::map<std::string, sdfPropertyType> sdfPropertyMap;
        for (const auto& property : sdf_object_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfPropertyMap.insert({property.key(), sdfProperty});
        }
        sdfObject.sdfProperty = sdfPropertyMap;
        std::cout << "Property List Size: " << sdfPropertyMap.size() << std::endl;
    }
    if (sdf_object_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdf_object_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
        sdfObject.sdfAction = sdfActionMap;
        std::cout << "Action List Size: " << sdfActionMap.size() << std::endl;
    }
    if (sdf_object_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdf_object_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
        sdfObject.sdfEvent = sdfEventMap;
        std::cout << "Event List Size: " << sdfEventMap.size() << std::endl;
    }
    if (sdf_object_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdf_object_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfObject.sdfData = sdfDataMap;
        std::cout << "Data List Size: " << sdfDataMap.size() << std::endl;
    }
    if (sdf_object_json.contains("minItems")){
        sdf_object_json.at("minItems").get_to(sdfObject.minItems);
        std::cout << "sdfObject minItems: " << sdfObject.minItems << std::endl;
    }
    if (sdf_object_json.contains("maxItems")){
        sdf_object_json.at("maxItems").get_to(sdfObject.maxItems);
        std::cout << "sdfObject maxItems: " << sdfObject.maxItems << std::endl;
    }
    return 0;
}

int parseSdfThing(const json& sdf_thing_json, sdfThingType& sdfThing)
{
    parseCommonQualities(sdf_thing_json, sdfThing);
    if (sdf_thing_json.contains("sdfObject")){
        std::map<std::string, sdfObjectType> sdfObjectMap;
        for (const auto& object : sdf_thing_json.at("sdfProperty").items()) {
            sdfObjectType sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdfObjectMap.insert({object.key(), sdfObject});
        }
        sdfThing.sdfObject = sdfObjectMap;
        std::cout << "Property List Size: " << sdfObjectMap.size() << std::endl;
    }
    if (sdf_thing_json.contains("sdfProperty")){
        std::map<std::string, sdfPropertyType> sdfPropertyMap;
        for (const auto& property : sdf_thing_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfPropertyMap.insert({property.key(), sdfProperty});
        }
        sdfThing.sdfProperty = sdfPropertyMap;
        std::cout << "Property List Size: " << sdfPropertyMap.size() << std::endl;
    }
    if (sdf_thing_json.contains("sdfAction")){
        std::map<std::string, sdfActionType> sdfActionMap;
        for (const auto& action : sdf_thing_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfActionMap.insert({action.key(), sdfAction});
        }
        sdfThing.sdfAction = sdfActionMap;
        std::cout << "Action List Size: " << sdfActionMap.size() << std::endl;
    }
    if (sdf_thing_json.contains("sdfEvent")){
        std::map<std::string, sdfEventType> sdfEventMap;
        for (const auto& event : sdf_thing_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfEventMap.insert({event.key(), sdfEvent});
        }
        sdfThing.sdfEvent = sdfEventMap;
        std::cout << "Event List Size: " << sdfEventMap.size() << std::endl;
    }
    if (sdf_thing_json.contains("sdfData")){
        sdfDataType sdfDataMap;
        for (const auto& data : sdf_thing_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfDataMap.insert({data.key(), sdfData});
        }
        sdfThing.sdfData = sdfDataMap;
        std::cout << "Data List Size: " << sdfDataMap.size() << std::endl;
    }
    if (sdf_thing_json.contains("minItems")){
        sdf_thing_json.at("minItems").get_to(sdfThing.minItems);
        std::cout << "sdfThing minItems: " << sdfThing.minItems << std::endl;
    }
    if (sdf_thing_json.contains("maxItems")){
        sdf_thing_json.at("maxItems").get_to(sdfThing.maxItems);
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

int parseInfoBlock(const json& info_block_json, infoBlockType& infoBlock)
{
    if (info_block_json.contains("title"))
        info_block_json.at("title").get_to(infoBlock.title);
    if (info_block_json.contains("description"))
        info_block_json.find("description").value().get_to(infoBlock.description);
    if (info_block_json.contains("version"))
        info_block_json.at("version").get_to(infoBlock.version);
    if (info_block_json.contains("modified"))
        info_block_json.at("modified").get_to(infoBlock.modified);
    if (info_block_json.contains("copyright"))
        info_block_json.at("copyright").get_to(infoBlock.copyright);
    if (info_block_json.contains("license"))
        info_block_json.at("license").get_to(infoBlock.license);
    if (info_block_json.contains("features"))
        info_block_json.at("features").get_to(infoBlock.features);
    if (info_block_json.contains("$comment"))
        info_block_json.at("$comment").get_to(infoBlock.$comment);
    return 0;
}

int parseSdfModel(const json& sdf_model_json, sdfModelType& sdfModel)
{
    if (sdf_model_json.contains("info")) {
        infoBlockType infoBlock;
        parseInfoBlock(sdf_model_json.at("info"), infoBlock);
        sdfModel.infoBlock = infoBlock;
    }

    if (sdf_model_json.contains("namespace")) {
        if (sdf_model_json.contains("defaultNamespace")) {
            namespaceType nsp;
            parseNamespaceBlock(sdf_model_json.at("namespace"), nsp);
            sdf_model_json.at("defaultNamespace").get_to(nsp.defaultNamespace);
            sdfModel.namespaceBlock = nsp;
        }
    }

    // Check if the sdf-model contains a sdfThing
    if (sdf_model_json.contains("sdfThing")){
        std::map<std::string, sdfThingType>sdfThingMap;
        sdfThingType sdfThing;
        parseSdfThing(sdf_model_json.at("sdfThing"), sdfThing);
    }
    // If not, check if the sdf-model contains a sdfObject
    else if (sdf_model_json.contains("sdfObject")) {
        sdfObjectType sdfObject;
        parseSdfObject(sdf_model_json.at("sdfObject"), sdfObject);
    }
    else {
        return -1;
    }
    return 0;
}

int parseSdfMapping(const json& sdf_mapping_json, sdfMappingType& sdfMapping)
{
    if (sdf_mapping_json.contains("info")) {
        infoBlockType infoBlock;
        parseInfoBlock(sdf_mapping_json.at("info"), infoBlock);
        sdfMapping.infoBlock = infoBlock;
    }
    if (sdf_mapping_json.contains("namespace")) {
        if (sdf_mapping_json.contains("defaultNamespace")) {
            namespaceType nsp;
            parseNamespaceBlock(sdf_mapping_json.at("namespace"), nsp);
            sdf_mapping_json.at("defaultNamespace").get_to(nsp.defaultNamespace);
            sdfMapping.namespaceBlock = nsp;
        }
    }
    if (sdf_mapping_json.contains("map")) {
        //TODO: Currently not sure how to implement different types and custom fields
        // Iterate through all items in the map section
        for (auto& mapping : sdf_mapping_json.at("map").items()) {
            auto& link = mapping.key();
            pugi::xml_node current_node = mappingTree.root();
            int j = 0;
            // Disassemble the JSON reference into its substrings and add them into a tree structure
            for (int i = 0; i < link.size(); i++){
                if (link[i] == '/') {
                    mappingTree.append_child(link.substr(j, i).c_str());
                    j = i;
                }
            }
            // Iterate through all items available at this JSON reference
            for (auto& items : mapping.value().items()){
                // Add a new attribute with its matching value
                mappingTree.append_attribute(items.key().c_str());
                //mappingTree.attribute(items.key().c_str()).set_value(items.value().get_to(char*));
            }

            std::cout << mappingTree.path() << std::endl;
        }
    }
    return 0;
}

int serializeCommonQualities(const commonQualityType& commonQuality, json& common_quality_json)
{
    if (!commonQuality.description.empty())
        common_quality_json["description"] = commonQuality.description;
    if (!commonQuality.label.empty())
        common_quality_json["label"] = commonQuality.label;
    if (!commonQuality.$comment.empty())
        common_quality_json["$comment"] = commonQuality.$comment;
    if (!commonQuality.sdfRef.empty())
        common_quality_json["sdfRef"] = commonQuality.sdfRef;
    if (!commonQuality.sdfRequired.empty())
        common_quality_json["sdfRequired"] = commonQuality.sdfRequired;
    return 0;
}

// Function prototype for serializeDataQualities
int serializeDataQualities(const dataQualityType& dataQuality, json& data_quality_json);

int serializeDataQualities(const dataQualityType& dataQuality, json& data_quality_json)
{
    // Serialize common qualities
    serializeCommonQualities(dataQuality, data_quality_json);

    // Serialize the data quality specific fields
    if (!dataQuality.type.empty())
        data_quality_json["type"] = dataQuality.type;
    if (!dataQuality.sdfChoice.empty()){
        json sdf_choice_map_json;
        for (const auto& sdf_choice_map : dataQuality.sdfChoice) {
            json sdf_choice_json;
            serializeDataQualities(sdf_choice_map.second, sdf_choice_json);
            sdf_choice_map_json[sdf_choice_map.first] = sdf_choice_json;
        }
        data_quality_json["sdfChoice"] = sdf_choice_map_json;
    }
    //enum_
    if (!dataQuality.const_.empty())
        data_quality_json["const"] = dataQuality.const_;
    if (!dataQuality.default_.empty())
        data_quality_json["default"] = dataQuality.default_;
    //minimum
    //maximum
    //exclusiveMinimum
    //exclusiveMaximum
    //multipleOf
    //minLength
    //maxLength
    if (!dataQuality.pattern.empty())
        data_quality_json["pattern"] = dataQuality.pattern;
    if (!dataQuality.format.empty())
        data_quality_json["format"] = dataQuality.format;
    //minItems
    //maxItems
    //uniqueItems
    //items
    if (!dataQuality.unit.empty())
        data_quality_json["unit"] = dataQuality.unit;
    //nullable
    if (!dataQuality.sdfType.empty())
        data_quality_json["sdfType"] = dataQuality.sdfType;
    if (!dataQuality.contentFormat.empty())
        data_quality_json["contentFormat"] = dataQuality.contentFormat;
    return 0;
}

int serializeSdfData(const sdfDataType& sdfData, json& sdf_data_json)
{
    for (const auto& data_quality_map : sdfData){
        json data_quality_json;
        serializeDataQualities(data_quality_map.second, data_quality_json);
        sdf_data_json[data_quality_map.first] = data_quality_json;
    }

    return 0;
}

int serializeSdfEvent(const sdfEventType& sdfEvent, json& sdf_event_json)
{
    // Serialize common qualities
    serializeCommonQualities(sdfEvent,sdf_event_json);

    // Serialize the sdfEvent specific fields
    json sdf_output_data_json;
    serializeDataQualities(sdfEvent.sdfOutputData, sdf_output_data_json);
    sdf_event_json["sdfOutputData"] = sdf_output_data_json;

    json sdf_data_json;
    serializeSdfData(sdfEvent.sdfData, sdf_data_json);
    sdf_event_json["sdfData"] = sdf_data_json;

    return 0;
}

int serializeSdfAction(const sdfActionType& sdfAction, json& sdf_action_json)
{
    // Serialize common qualities
    serializeCommonQualities(sdfAction, sdf_action_json);

    // Serialize the sdfAction specific fields
    //TODO: Check if these can be empty
    json sdf_input_data_json;
    serializeDataQualities(sdfAction.sdfInputData, sdf_input_data_json);
    if (!sdf_input_data_json.is_null())
        sdf_action_json["sdfInputData"] = sdf_input_data_json;

    json sdf_output_data_json;
    serializeDataQualities(sdfAction.sdfOutputData, sdf_output_data_json);
    if (!sdf_output_data_json.is_null())
        sdf_action_json["sdfOutputData"] = sdf_output_data_json;

    json sdf_data_json;
    serializeSdfData(sdfAction.sdfData, sdf_data_json);
    if (!sdf_data_json.is_null())
        sdf_action_json["sdfData"] = sdf_data_json;

    return 0;
}

int serializeSdfProperty(const sdfPropertyType& sdfProperty, json& sdf_property_json)
{
    // Serialize data qualities
    serializeDataQualities(sdfProperty, sdf_property_json);

    // Serialize the sdfProperty specific fields
    if (sdfProperty.readable.has_value())
        sdf_property_json["readable"] = sdfProperty.readable.value();
    if (sdfProperty.writable.has_value())
        sdf_property_json["writable"] = sdfProperty.writable.value();
    if (sdfProperty.observable.has_value())
        sdf_property_json["observable"] = sdfProperty.observable.value();

    return 0;
}

int serializeSdfObject(const sdfObjectType& sdfObject, json& sdf_object_json)
{
    // Serialize the common qualities
    serializeCommonQualities(sdfObject, sdf_object_json);

    // Serialize the sdfObject specific fields
    // Serialize the sdfProperties
    if (!sdfObject.sdfProperty.empty()) {
        json sdf_property_map_json;
        for (const auto& sdf_property_map: sdfObject.sdfProperty) {
            json sdf_property_json;
            serializeSdfProperty(sdf_property_map.second, sdf_property_json);
            sdf_property_map_json[sdf_property_map.first] = sdf_property_json;
        }
        sdf_object_json["sdfProperty"] = sdf_property_map_json;
    }

    // Serialize the sdfActions
    if (!sdfObject.sdfAction.empty()) {
        json sdf_action_map_json;
        for (const auto& sdf_action_map: sdfObject.sdfAction) {
            json sdf_action_json;
            serializeSdfAction(sdf_action_map.second, sdf_action_json);
            sdf_action_map_json[sdf_action_map.first] = sdf_action_json;
        }
        sdf_object_json["sdfAction"] = sdf_action_map_json;
    }

    // Serialize the sdfEvents
    if (!sdfObject.sdfEvent.empty()) {
        json sdf_event_map_json;
        for (const auto& sdf_event_map: sdfObject.sdfEvent) {
            json sdf_event_json;
            serializeSdfEvent(sdf_event_map.second, sdf_event_json);
            sdf_event_map_json[sdf_event_map.first] = sdf_event_json;
        }
        sdf_object_json["sdfEvent"] = sdf_event_map_json;
    }
    if (!sdfObject.sdfData.empty()) {
        json sdf_data_json;
        // serializeSdfData(sdfObject.sdfData, sdf_data_json);
        sdf_object_json["sdfData"] = sdf_data_json;
    }
    //minItems
    //maxItems
    return 0;
}

int serializeSdfThing(const sdfThingType& sdfThing, json& sdf_thing_json)
{
    // Serialize the common qualities
    serializeCommonQualities(sdfThing, sdf_thing_json);

    // Serialize the sdfThing specific fields
    // Serialize the sdfObjects
    if (!sdfThing.sdfObject.empty()) {
        json sdf_object_map_json;
        for (const auto& sdf_object_map: sdfThing.sdfObject) {
            json sdf_object_json;
            serializeSdfObject(sdf_object_map.second, sdf_object_json);
            sdf_object_map_json[sdf_object_map.first] = sdf_object_json;
        }
        sdf_thing_json["sdfObject"] = sdf_object_map_json;
    }

    // Serialize the sdfProperties
    if (!sdfThing.sdfProperty.empty()) {
        json sdf_property_map_json;
        for (const auto& sdf_property_map: sdfThing.sdfProperty) {
            json sdf_property_json;
            serializeSdfProperty(sdf_property_map.second, sdf_property_json);
            sdf_property_map_json[sdf_property_map.first] = sdf_property_json;
        }
        sdf_thing_json["sdfProperty"] = sdf_property_map_json;
    }

    // Serialize the sdfActions
    if (!sdfThing.sdfAction.empty()) {
        json sdf_action_map_json;
        for (const auto& sdf_action_map: sdfThing.sdfAction) {
            json sdf_action_json;
            serializeSdfAction(sdf_action_map.second, sdf_action_json);
            sdf_action_map_json[sdf_action_map.first] = sdf_action_json;
        }
        sdf_thing_json["sdfAction"] = sdf_action_map_json;
    }

    // Serialize the sdfEvents
    if (!sdfThing.sdfEvent.empty()) {
        json sdf_event_map_json;
        for (const auto& sdf_event_map: sdfThing.sdfEvent) {
            json sdf_event_json;
            serializeSdfEvent(sdf_event_map.second, sdf_event_json);
            sdf_event_map_json[sdf_event_map.first] = sdf_event_json;
        }
        sdf_thing_json["sdfEvent"] = sdf_event_map_json;
    }
    if (!sdfThing.sdfData.empty()) {
        json sdf_data_json;
        serializeSdfData(sdfThing.sdfData, sdf_data_json);
        sdf_thing_json["sdfData"] = sdf_data_json;
    }
    //minItems
    //maxItems
    return 0;

}

int serializeNamespaceBlock(const namespaceType& namespaceBlock, json& namespace_block_json)
{
    if (!namespaceBlock.namespaces.empty())
        namespace_block_json["namespaces"] = namespaceBlock.namespaces;
    if (!namespaceBlock.defaultNamespace.empty())
        namespace_block_json["defaultNamespace"] = namespaceBlock.defaultNamespace;
    return 0;
}

int serializeInfoBlock(const infoBlockType& infoBlock, json& info_block_json)
{
    if (!infoBlock.title.empty())
        info_block_json["title"] = infoBlock.title;
    if (!infoBlock.description.empty())
        info_block_json["description"] = infoBlock.description;
    if (!infoBlock.version.empty())
        info_block_json["version"] = infoBlock.version;
    if (!infoBlock.modified.empty())
        info_block_json["modified"] = infoBlock.modified;
    if (!infoBlock.copyright.empty())
        info_block_json["copyright"] = infoBlock.copyright;
    if (!infoBlock.license.empty())
        info_block_json["license"] = infoBlock.license;
    if (!infoBlock.features.empty())
        info_block_json["features"] = infoBlock.features;
    if (!infoBlock.$comment.empty())
        info_block_json["$comment"] = infoBlock.$comment;

    return 0;
}

int serializeSdfModel(const sdfModelType& sdfModel, json& sdf_model_json)
{
    // Serialize the information block and add it to info
    json info_block_json;
    serializeInfoBlock(sdfModel.infoBlock, info_block_json);
    sdf_model_json["info"] = info_block_json;

    // Serialize the namespace information and append it
    serializeNamespaceBlock(sdfModel.namespaceBlock, sdf_model_json);

    if (sdfModel.sdfThing.has_value()){
        json sdf_thing_json;
        serializeSdfThing(sdfModel.sdfThing.value(), sdf_thing_json);
        sdf_model_json["sdfThing"] = sdf_thing_json;
    }
    else if (sdfModel.sdfObject.has_value()){
        json sdf_object_json;
        serializeSdfObject(sdfModel.sdfObject.value(), sdf_object_json);
        sdf_model_json["sdfObject"] = sdf_object_json;
    }
    else {
        // If neither sdfThing nor sdfObject are present, the models definition block is empty and thus invalid
        return -1;
    }

    return 0;
}

int serializeSdfMapping(const sdfMappingType& sdfMapping, json& sdf_mapping_json)
{
    return 0;
}