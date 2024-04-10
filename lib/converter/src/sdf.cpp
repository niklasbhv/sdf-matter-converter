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
    if (common_qualities_json.contains("description"))
        common_qualities_json.at("description").get_to(commonQuality.description);

    if (common_qualities_json.contains("label"))
        common_qualities_json.at("label").get_to(commonQuality.label);

    if (common_qualities_json.contains("$comment"))
        common_qualities_json.at("$comment").get_to(commonQuality.$comment);

    if (common_qualities_json.contains("sdfRef"))
        common_qualities_json.at("sdfRef").get_to(commonQuality.sdfRef);

    if (common_qualities_json.contains("sdfRequired"))
        common_qualities_json.at("sdfRequired").get_to(commonQuality.sdfRequired);

    return 0;
}

// Function prototype for parseDataQualities
int parseDataQualities(const json& data_qualities_json, dataQualityType& dataQuality);

int parseSdfChoice(const json& sdf_choice_json, std::map<std::string, dataQualityType>& sdfChoiceMap)
{
    for (const auto& choice : sdf_choice_json.items()){
        dataQualityType dataQuality;
        parseDataQualities(choice.value(), dataQuality);
        sdfChoiceMap.insert({choice.key(), dataQuality});
    }

    return 0;
}

int parseDataQualities(const json& data_qualities_json, dataQualityType& dataQuality)
{
    parseCommonQualities(data_qualities_json, dataQuality);

    if (data_qualities_json.contains("unit"))
        data_qualities_json.at("unit").get_to(dataQuality.unit);

    if (data_qualities_json.contains("nullable"))
        data_qualities_json.at("nullable").get_to(dataQuality.nullable);

    if (data_qualities_json.contains("contentFormat"))
        data_qualities_json.at("contentFormat").get_to(dataQuality.contentFormat);

    if (data_qualities_json.contains("sdfType"))
        data_qualities_json.at("sdfType").get_to(dataQuality.sdfType);

    if (data_qualities_json.contains("sdfChoice"))
        parseSdfChoice(data_qualities_json.at("sdfChoice"), dataQuality.sdfChoice);

    if (data_qualities_json.contains("enum"))
        data_qualities_json.at("enum").get_to(dataQuality.enum_);

    return 0;
}

int parseSdfEvent(const json& sdf_event_json, sdfEventType& sdfEvent)
{
    parseCommonQualities(sdf_event_json, sdfEvent);

    if (sdf_event_json.contains("sdfOutputData"))
        parseDataQualities(sdf_event_json.at("sdfOutputData"), sdfEvent.sdfOutputData);

    if (sdf_event_json.contains("sdfData")){
        for (const auto& data : sdf_event_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfEvent.sdfData.insert({data.key(), sdfData});
        }
    }

    return 0;
}

int parseSdfAction(const json& sdf_action_json, sdfActionType& sdfAction)
{
    parseCommonQualities(sdf_action_json, sdfAction);

    if (sdf_action_json.contains("sdfInputData"))
        parseDataQualities(sdf_action_json.at("sdfInputData"), sdfAction.sdfInputData);

    if (sdf_action_json.contains("sdfOutputData"))
        parseDataQualities(sdf_action_json.at("sdfOutputData"), sdfAction.sdfOutputData);

    if (sdf_action_json.contains("sdfData")){
        for (const auto& data : sdf_action_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfAction.sdfData.insert({data.key(), sdfData});
        }
    }

    return 0;
}

int parseSdfProperty(const json& sdf_property_json, sdfPropertyType& sdfProperty)
{
    parseDataQualities(sdf_property_json, sdfProperty);
    //TODO: Currently broken because of optional
    //if (sdf_property_json.contains("readable"))
        //sdf_property_json.at("readable").get_to(sdfProperty.readable);

    //if (sdf_property_json.contains("writable"))
        //sdf_property_json.at("writable").get_to(sdfProperty.writable);

    //if (sdf_property_json.contains("observable"))
        //sdf_property_json.at("observable").get_to(sdfProperty.observable);

    return 0;
}

int parseSdfObject(const json& sdf_object_json, sdfObjectType& sdfObject)
{
    parseCommonQualities(sdf_object_json, sdfObject);

    if (sdf_object_json.contains("sdfProperty")){
        for (const auto& property : sdf_object_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfObject.sdfProperty.insert({property.key(), sdfProperty});
        }
    }

    if (sdf_object_json.contains("sdfAction")){
        for (const auto& action : sdf_object_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfObject.sdfAction.insert({action.key(), sdfAction});
        }
    }

    if (sdf_object_json.contains("sdfEvent")){
        for (const auto& event : sdf_object_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfObject.sdfEvent.insert({event.key(), sdfEvent});
        }
    }

    if (sdf_object_json.contains("sdfData")){
        for (const auto& data : sdf_object_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfObject.sdfData.insert({data.key(), sdfData});
        }
    }

    if (sdf_object_json.contains("minItems"))
        sdf_object_json.at("minItems").get_to(sdfObject.minItems);

    if (sdf_object_json.contains("maxItems"))
        sdf_object_json.at("maxItems").get_to(sdfObject.maxItems);

    return 0;
}

int parseSdfThing(const json& sdf_thing_json, sdfThingType& sdfThing)
{
    parseCommonQualities(sdf_thing_json, sdfThing);

    if (sdf_thing_json.contains("sdfObject")){
        for (const auto& object : sdf_thing_json.at("sdfObject").items()) {
            sdfObjectType sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdfThing.sdfObject.insert({object.key(), sdfObject});
        }
    }

    if (sdf_thing_json.contains("sdfProperty")){
        for (const auto& property : sdf_thing_json.at("sdfProperty").items()) {
            sdfPropertyType sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfThing.sdfProperty.insert({property.key(), sdfProperty});
        }
    }

    if (sdf_thing_json.contains("sdfAction")){
        for (const auto& action : sdf_thing_json.at("sdfAction").items()) {
            sdfActionType sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfThing.sdfAction.insert({action.key(), sdfAction});
        }
    }

    if (sdf_thing_json.contains("sdfEvent")){
        for (const auto& event : sdf_thing_json.at("sdfEvent").items()) {
            sdfEventType sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfThing.sdfEvent.insert({event.key(), sdfEvent});
        }
    }

    if (sdf_thing_json.contains("sdfData")){
        for (const auto& data : sdf_thing_json.at("sdfData").items()) {
            dataQualityType sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfThing.sdfData.insert({data.key(), sdfData});
        }
    }

    if (sdf_thing_json.contains("minItems"))
        sdf_thing_json.at("minItems").get_to(sdfThing.minItems);

    if (sdf_thing_json.contains("maxItems"))
        sdf_thing_json.at("maxItems").get_to(sdfThing.maxItems);

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
    if (sdf_model_json.contains("info"))
        parseInfoBlock(sdf_model_json.at("info"), sdfModel.infoBlock);

    if (sdf_model_json.contains("namespace")) {
        if (sdf_model_json.contains("defaultNamespace")) {
            parseNamespaceBlock(sdf_model_json.at("namespace"), sdfModel.namespaceBlock);
            sdf_model_json.at("defaultNamespace").get_to(sdfModel.namespaceBlock.defaultNamespace);
        }
    }

    // Check if the sdf-model contains a sdfThing
    if (sdf_model_json.contains("sdfThing")){
        sdfThingType sdfThing;
        parseSdfThing(sdf_model_json.at("sdfThing").front(), sdfThing);
        sdfModel.sdfThing = sdfThing;
    }

    // If not, check if the sdf-model contains a sdfObject
    else if (sdf_model_json.contains("sdfObject")) {
        sdfObjectType sdfObject;
        parseSdfObject(sdf_model_json.at("sdfObject").front(), sdfObject);
        sdfModel.sdfObject = sdfObject;
    }

    else {
        return -1;
    }

    return 0;
}

int parseSdfMapping(const json& sdf_mapping_json, sdfMappingType& sdfMapping)
{
    if (sdf_mapping_json.contains("info")) {
        parseInfoBlock(sdf_mapping_json.at("info"), sdfMapping.infoBlock);
    }

    if (sdf_mapping_json.contains("namespace")) {
        if (sdf_mapping_json.contains("defaultNamespace")) {
            parseNamespaceBlock(sdf_mapping_json.at("namespace"), sdfMapping.namespaceBlock);
            sdf_mapping_json.at("defaultNamespace").get_to(sdfMapping.namespaceBlock.defaultNamespace);
        }
    }

    if (sdf_mapping_json.contains("map")) {
        for (const auto& reference : sdf_mapping_json.at("map").items()) {
            for (const auto& field : reference.value().items()) {
                sdfMapping.map[reference.key()] = {{field.key(), field.value()}};
            }
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

//! String to bool helper
bool to_bool(std::string const& s) {
    return s != "0";
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
    if (!dataQuality.enum_.empty())
        data_quality_json["enum"] = dataQuality.enum_;
    if (!dataQuality.const_.empty())
        data_quality_json["const"] = dataQuality.const_;
    //TODO: Maybe write a generic helper to cast this
    if (!dataQuality.default_.empty()) {
        if (dataQuality.type == "boolean")
            data_quality_json["default"] = to_bool(dataQuality.default_);
        else if (dataQuality.type == "number") {
            float default_ = std::stof(dataQuality.default_);
            data_quality_json["default"] = default_;
        } else if (dataQuality.type == "integer") {
            int default_ = std::stoi(dataQuality.default_);
            data_quality_json["default"] = default_;
        } else {
            //TODO: Does array have to be handled differently
            data_quality_json["default"] = dataQuality.default_;
        }
    }
    if (dataQuality.minimum.has_value())
        data_quality_json["minimum"] = dataQuality.minimum.value();
    if (dataQuality.maximum.has_value())
        data_quality_json["maximum"] = dataQuality.maximum.value();
    if (dataQuality.exclusiveMinimum.has_value())
        data_quality_json["exclusiveMinimum"] = dataQuality.exclusiveMinimum.value();
    if (dataQuality.exclusiveMaximum.has_value())
        data_quality_json["exclusiveMaximum"] = dataQuality.exclusiveMaximum.value();
    if (dataQuality.multipleOf.has_value())
        data_quality_json["multipleOf"] = dataQuality.multipleOf.value();
    if (dataQuality.minLength.has_value())
        data_quality_json["minLength"] = dataQuality.minLength.value();
    if (dataQuality.maxLength.has_value())
        data_quality_json["maxLength"] = dataQuality.maxLength.value();
    if (!dataQuality.pattern.empty())
        data_quality_json["pattern"] = dataQuality.pattern;
    if (!dataQuality.format.empty())
        data_quality_json["format"] = dataQuality.format;
    if (dataQuality.minItems.has_value())
        data_quality_json["minItems"] = dataQuality.minItems.value();
    if (dataQuality.maxItems.has_value())
        data_quality_json["maxItems"] = dataQuality.maxItems.value();
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
        namespace_block_json["namespace"] = namespaceBlock.namespaces;
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
        json sdf_thing_map_json;
        sdf_thing_map_json[sdfModel.sdfThing.value().label] = sdf_thing_json;
        sdf_model_json["sdfThing"] = sdf_thing_map_json;
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
    // Serialize the information block and add it to info
    json info_block_json;
    serializeInfoBlock(sdfMapping.infoBlock, info_block_json);
    sdf_mapping_json["info"] = info_block_json;

    // Serialize the namespace information and append it
    serializeNamespaceBlock(sdfMapping.namespaceBlock, sdf_mapping_json);

    // Serialize the mapping section
    sdf_mapping_json["map"] = sdfMapping.map;
    return 0;
}