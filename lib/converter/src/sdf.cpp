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

#include <string>

#include <nlohmann/json.hpp>

#include "matter.h"
#include "sdf.h"

using json = nlohmann::ordered_json;

namespace sdf {

/*
 * Reference to the SDF-Model used for resolving sdf_ref-Elements.
 */
json global_sdf_model = {};

/*
 * Function used to resolve sdf_ref qualities.
 */
int resolveSdfRef(json& sdf_ref_qualities_json)
{
    std::string sdfRef;
    json sdf_ref_qualities_json_copy = sdf_ref_qualities_json;
    sdf_ref_qualities_json.at("sdf_ref").get_to(sdfRef);
    json patch = global_sdf_model[json::json_pointer(sdfRef.substr(1))];
    patch.merge_patch(sdf_ref_qualities_json_copy);
    sdf_ref_qualities_json = patch;

    return 0;
}

/*
 * Parse common qualities from json into a CommonQuality object.
 */
int parseCommonQualities(json& common_qualities_json, CommonQuality& commonQuality)
{
    // If a sdf_ref-Element exists, resolve it
    if (common_qualities_json.contains("sdf_ref"))
        resolveSdfRef(common_qualities_json);

    if (common_qualities_json.contains("description"))
        common_qualities_json.at("description").get_to(commonQuality.description);

    if (common_qualities_json.contains("label"))
        common_qualities_json.at("label").get_to(commonQuality.label);

    if (common_qualities_json.contains("comment"))
        common_qualities_json.at("comment").get_to(commonQuality.comment);

    if (common_qualities_json.contains("sdf_required"))
        common_qualities_json.at("sdf_required").get_to(commonQuality.sdf_required);

    return 0;
}

/*
 * Function prototype used for recursive calls.
 */
int parseDataQualities(json& data_qualities_json, DataQuality& dataQuality);

/*
 * Parse a sdf_choice from json into a DataQuality map.
 */
int parseSdfChoice(json& sdf_choice_json, std::map<std::string, DataQuality>& sdfChoiceMap)
{
    // Iterate through all sdf_choice items and parse them individually
    for (const auto& choice : sdf_choice_json.items()){
        DataQuality dataQuality;
        parseDataQualities(choice.value(), dataQuality);
        sdfChoiceMap.insert({choice.key(), dataQuality});
    }

    return 0;
}

/*
 * Parse a JSO item type from json into a JsoItemType object.
 */
int parseJsoItemType(json& jso_item_type_json, JsoItem& jsoItem)
{
    // Parse the common qualities
    if (jso_item_type_json.contains("sdf_ref"))
        resolveSdfRef(jso_item_type_json);

    if (jso_item_type_json.contains("description"))
        jso_item_type_json.at("description").get_to(jsoItem.description);

    if (jso_item_type_json.contains("comment"))
        jso_item_type_json.at("comment").get_to(jsoItem.comment);

    // Parse general qualities
    if (jso_item_type_json.contains("type"))
        jso_item_type_json.at("type").get_to(jsoItem.type);

    if (jso_item_type_json.contains("sdf_choice"))
        parseSdfChoice(jso_item_type_json.at("sdf_choice"), jsoItem.sdf_choice);

    if (jso_item_type_json.contains("enum"))
        jso_item_type_json.at("enum").get_to(jsoItem.enum_);

    // Parse number and integer qualities
    if (jso_item_type_json.contains("minimum"))
        jso_item_type_json.at("minimum").get_to(jsoItem.minimum);

    if (jso_item_type_json.contains("maximum"))
        jso_item_type_json.at("maximum").get_to(jsoItem.maximum);

    // Parse string qualities
    if (jso_item_type_json.contains("format"))
        jso_item_type_json.at("format").get_to(jsoItem.format);

    if (jso_item_type_json.contains("min_length"))
        jso_item_type_json.at("min_length").get_to(jsoItem.min_length);

    if (jso_item_type_json.contains("max_length"))
        jso_item_type_json.at("max_length").get_to(jsoItem.max_length);

    // Parse object qualities
    // Iterate through all items inside properties and parse them individually
    if (jso_item_type_json.contains("properties")){
        for (const auto& data : jso_item_type_json.at("properties").items()) {
            DataQuality sdfData;
            parseDataQualities(data.value(), sdfData);
            jsoItem.properties.insert({data.key(), sdfData});
        }
    }
    if (jso_item_type_json.contains("required"))
        jso_item_type_json.at("required").get_to(jsoItem.required);

    return 0;
}

/*
 * Parse data qualities from json into a DataQuality object.
 */
int parseDataQualities(json& data_qualities_json, DataQuality& dataQuality)
{
    // Parse the common qualities
    parseCommonQualities(data_qualities_json, dataQuality);

    // Parse general qualities
    if (data_qualities_json.contains("type"))
        data_qualities_json.at("type").get_to(dataQuality.type);

    if (data_qualities_json.contains("sdf_choice"))
        parseSdfChoice(data_qualities_json.at("sdf_choice"), dataQuality.sdf_choice);

    if (data_qualities_json.contains("enum"))
        data_qualities_json.at("enum").get_to(dataQuality.enum_);

    // Select a fitting datatype for the default quality based on the set json type
    if (data_qualities_json.contains("const")) {
        // TODO: Can the object type have a default value?
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("const").get_to(number);
            dataQuality.default_ = number;
        } else if (dataQuality.type == "string") {
            std::string string;
            data_qualities_json.at("const").get_to(string);
            dataQuality.default_ = string;
        } else if (dataQuality.type == "boolean") {
            bool boolean;
            data_qualities_json.at("const").get_to(boolean);
            dataQuality.default_ = boolean;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            uint64_t integer;
            data_qualities_json.at("const").get_to(integer);
            dataQuality.default_ = integer;
        } else if (dataQuality.type == "array") {
            //data_qualities_json.at("const").get_to(dataQuality.default_->array);
        } else if (dataQuality.type == "object") {
            //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        }
    }

    // Select a fitting datatype for the default quality based on the set json type
    if (data_qualities_json.contains("default")) {
        // TODO: Can the object type have a default value?
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("default").get_to(number);
            dataQuality.default_ = number;
        } else if (dataQuality.type == "string") {
            std::string string;
            data_qualities_json.at("default").get_to(string);
            dataQuality.default_ = string;
        } else if (dataQuality.type == "boolean") {
            bool boolean;
            data_qualities_json.at("default").get_to(boolean);
            dataQuality.default_ = boolean;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            uint64_t integer;
            data_qualities_json.at("default").get_to(integer);
            dataQuality.default_ = integer;
        } else if (dataQuality.type == "array") {
            //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        } else if (dataQuality.type == "object") {
            //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        }
    }

    // Parse number and integer qualities
    if (data_qualities_json.contains("minimum")) {
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("minimum").get_to(number);
            dataQuality.minimum = number;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            int integer;
            data_qualities_json.at("minimum").get_to(integer);
            dataQuality.minimum = integer;
        }
    }

    if (data_qualities_json.contains("maximum")) {
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("maximum").get_to(number);
            dataQuality.maximum = number;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            int integer;
            data_qualities_json.at("maximum").get_to(integer);
            dataQuality.maximum = integer;
        }
    }

    if (data_qualities_json.contains("exclusive_minimum")) {
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("exclusive_minimum").get_to(number);
            dataQuality.exclusive_minimum = number;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            int integer;
            data_qualities_json.at("exclusive_minimum").get_to(integer);
            dataQuality.exclusive_minimum = integer;
        }
    }

    if (data_qualities_json.contains("exclusive_maximum")) {
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("exclusive_maximum").get_to(number);
            dataQuality.exclusive_maximum = number;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            int integer;
            data_qualities_json.at("exclusive_maximum").get_to(integer);
            dataQuality.exclusive_maximum = integer;
        }
    }

    if (data_qualities_json.contains("multiple_of")) {
        if (dataQuality.type == "number") {
            double number;
            data_qualities_json.at("multiple_of").get_to(number);
            dataQuality.multiple_of = number;
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            int integer;
            data_qualities_json.at("multiple_of").get_to(integer);
            dataQuality.multiple_of = integer;
        }
    }

    // Parse string qualities
    if (data_qualities_json.contains("min_length"))
        data_qualities_json.at("min_length").get_to(dataQuality.min_length);

    if (data_qualities_json.contains("max_length"))
        data_qualities_json.at("max_length").get_to(dataQuality.max_length);

    if (data_qualities_json.contains("pattern"))
        data_qualities_json.at("pattern").get_to(dataQuality.pattern);

    if (data_qualities_json.contains("format"))
        data_qualities_json.at("format").get_to(dataQuality.format);

    // Parse array qualities
    if (data_qualities_json.contains("min_items"))
        data_qualities_json.at("min_items").get_to(dataQuality.min_items);

    if (data_qualities_json.contains("max_items"))
        data_qualities_json.at("max_items").get_to(dataQuality.max_items);

    if (data_qualities_json.contains("unique_items"))
        data_qualities_json.at("unique_items").get_to(dataQuality.unique_items);

    if (data_qualities_json.contains("items")) {
        JsoItem jsoItem;
        parseJsoItemType(data_qualities_json.at("items"), jsoItem);
        dataQuality.items = jsoItem;
    }

    // Parse object qualities
    // Iterate through all items inside properties and parse them individually
    if (data_qualities_json.contains("properties")){
        for (const auto& data : data_qualities_json.at("properties").items()) {
            DataQuality sdfData;
            parseDataQualities(data.value(), sdfData);
            dataQuality.properties.insert({data.key(), sdfData});
        }
    }
    if (data_qualities_json.contains("required"))
        data_qualities_json.at("required").get_to(dataQuality.required);

    // Parse additional qualities
    if (data_qualities_json.contains("unit"))
        data_qualities_json.at("unit").get_to(dataQuality.unit);

    if (data_qualities_json.contains("nullable"))
        data_qualities_json.at("nullable").get_to(dataQuality.nullable);

    if (data_qualities_json.contains("sdf_type"))
        data_qualities_json.at("sdf_type").get_to(dataQuality.sdf_type);

    if (data_qualities_json.contains("content_format"))
        data_qualities_json.at("content_format").get_to(dataQuality.content_format);

    return 0;
}

/*
 * Parse a sdf_event from json into a SdfEvent object.
 */
int parseSdfEvent(json& sdf_event_json, SdfEvent& sdfEvent)
{
    // Parse the common qualities
    parseCommonQualities(sdf_event_json, sdfEvent);

    // Parse the remaining fields
    if (sdf_event_json.contains("sdf_output_data"))
        parseDataQualities(sdf_event_json.at("sdf_output_data"), sdfEvent.sdf_output_data);

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_event_json.contains("sdf_data")){
        for (const auto& data : sdf_event_json.at("sdf_data").items()) {
            DataQuality sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfEvent.sdf_data.insert({data.key(), sdfData});
        }
    }

    return 0;
}

/*
 * Parse a sdf_action from json into a SdfAction object.
 */
int parseSdfAction(json& sdf_action_json, SdfAction& sdfAction)
{
    // Parse the common qualities
    parseCommonQualities(sdf_action_json, sdfAction);

    // Parse the remaining fields
    if (sdf_action_json.contains("sdf_input_data")) {
        DataQuality sdfInputData;
        parseDataQualities(sdf_action_json.at("sdf_input_data"), sdfInputData);
        sdfAction.sdf_input_data = sdfInputData;
    }

    if (sdf_action_json.contains("sdf_output_data")) {
        DataQuality sdfOutputData;
        parseDataQualities(sdf_action_json.at("sdf_output_data"), sdfOutputData);
        sdfAction.sdf_output_data = sdfOutputData;
    }


    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_action_json.contains("sdf_data")){
        SdfData sdfData;
        for (const auto& data : sdf_action_json.at("sdf_data").items()) {
            DataQuality dataQuality;
            parseDataQualities(data.value(), dataQuality);
            sdfData.insert({data.key(), dataQuality});
        }
        sdfAction.sdf_data = sdfData;
    }

    return 0;
}

/*
 * Parse a sdf_property from json into a SdfProperty object.
 */
int parseSdfProperty(json& sdf_property_json, SdfProperty& sdfProperty)
{
    // Parse the data qualities
    parseDataQualities(sdf_property_json, sdfProperty);

    // Parse the remaining fields
    if (sdf_property_json.contains("readable"))
        sdf_property_json.at("readable").get_to(sdfProperty.readable);

    if (sdf_property_json.contains("writable"))
        sdf_property_json.at("writable").get_to(sdfProperty.writable);

    if (sdf_property_json.contains("observable"))
        sdf_property_json.at("observable").get_to(sdfProperty.observable);

    return 0;
}

/*
 * Parse a sdf_object from json into a SdfObject object.
 */
int parseSdfObject(json& sdf_object_json, SdfObject& sdfObject)
{
    // Parse the common qualities
    parseCommonQualities(sdf_object_json, sdfObject);

    // Iterate through all sdfProperties and parse them individually
    if (sdf_object_json.contains("sdf_property")){
        for (const auto& property : sdf_object_json.at("sdf_property").items()) {
            SdfProperty sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfObject.sdf_property.insert({property.key(), sdfProperty});
        }
    }

    // Iterate through all sdfActions and parse them individually
    if (sdf_object_json.contains("sdf_action")){
        for (const auto& action : sdf_object_json.at("sdf_action").items()) {
            SdfAction sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfObject.sdf_action.insert({action.key(), sdfAction});
        }
    }

    // Iterate through all sdfEvents and parse them individually
    if (sdf_object_json.contains("sdf_event")){
        for (const auto& event : sdf_object_json.at("sdf_event").items()) {
            SdfEvent sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfObject.sdf_event.insert({event.key(), sdfEvent});
        }
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_object_json.contains("sdf_data")){
        for (const auto& data : sdf_object_json.at("sdf_data").items()) {
            DataQuality sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfObject.sdf_data.insert({data.key(), sdfData});
        }
    }

    // Parse the remaining fields
    if (sdf_object_json.contains("min_items"))
        sdf_object_json.at("min_items").get_to(sdfObject.min_items);

    if (sdf_object_json.contains("max_items"))
        sdf_object_json.at("max_items").get_to(sdfObject.max_items);

    return 0;
}

/*
 * Parse a sdf_thing from json into a SdfThing object.
 */
int parseSdfThing(json& sdf_thing_json, SdfThing& sdfThing)
{
    // Parse the common qualities
    parseCommonQualities(sdf_thing_json, sdfThing);

    // sdf_thing

    // Iterate through all sdfObjects and parse them individually
    if (sdf_thing_json.contains("sdf_object")){
        for (const auto& object : sdf_thing_json.at("sdf_object").items()) {
            SdfObject sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdfThing.sdf_object.insert({object.key(), sdfObject});
        }
    }

    // Iterate through all sdfProperties and parse them individually
    if (sdf_thing_json.contains("sdf_property")){
        for (const auto& property : sdf_thing_json.at("sdf_property").items()) {
            SdfProperty sdfProperty;
            parseSdfProperty(property.value(), sdfProperty);
            sdfThing.sdf_property.insert({property.key(), sdfProperty});
        }
    }

    // Iterate through all sdfActions and parse them individually
    if (sdf_thing_json.contains("sdf_action")){
        for (const auto& action : sdf_thing_json.at("sdf_action").items()) {
            SdfAction sdfAction;
            parseSdfAction(action.value(), sdfAction);
            sdfThing.sdf_action.insert({action.key(), sdfAction});
        }
    }

    // Iterate through all sdfEvents and parse them individually
    if (sdf_thing_json.contains("sdf_event")){
        for (const auto& event : sdf_thing_json.at("sdf_event").items()) {
            SdfEvent sdfEvent;
            parseSdfEvent(event.value(), sdfEvent);
            sdfThing.sdf_event.insert({event.key(), sdfEvent});
        }
    }

    // Iterate through all items inside sdf_data and parse them individually
    if (sdf_thing_json.contains("sdf_data")){
        for (const auto& data : sdf_thing_json.at("sdf_data").items()) {
            DataQuality sdfData;
            parseDataQualities(data.value(), sdfData);
            sdfThing.sdf_data.insert({data.key(), sdfData});
        }
    }

    // Parse the remaining fields
    if (sdf_thing_json.contains("min_items"))
        sdf_thing_json.at("min_items").get_to(sdfThing.min_items);

    if (sdf_thing_json.contains("max_items"))
        sdf_thing_json.at("max_items").get_to(sdfThing.max_items);

    return 0;
}

/*
 * Parse a namespace block from json into a NamespaceBlock object.
 */
int parseNamespaceBlock(json& namespace_json, NamespaceBlock& namespace_)
{
    // Iterate through all namespace items and parse them individually
    for (const auto& nsp_item : namespace_json.items()) {
        namespace_.namespaces.insert({nsp_item.key(), nsp_item.value()});
    }

    return 0;
}

/*
 * Parse an information block from json into a InformationBlock object.
 */
int parseInfoBlock(json& info_block_json, InformationBlock& infoBlock)
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

    if (info_block_json.contains("comment"))
        info_block_json.at("comment").get_to(infoBlock.comment);

    return 0;
}

/*
 * Parse a sdf-model from json into a SdfModel object.
 */
int parseSdfModel(json& sdf_model_json, SdfModel& sdf_model)
{
    // Set the global sdf_model reference
    global_sdf_model = sdf_model_json;

    // Parse the information block
    if (sdf_model_json.contains("info"))
        parseInfoBlock(sdf_model_json.at("info"), sdf_model.information_block);

    // Parse the namespace block
    if (sdf_model_json.contains("namespace")) {
        if (sdf_model_json.contains("default_namespace")) {
            parseNamespaceBlock(sdf_model_json.at("namespace"), sdf_model.namespace_block);
            sdf_model_json.at("default_namespace").get_to(sdf_model.namespace_block.default_namespace);
        }
    }

    // Parse the sdfThings
    if (sdf_model_json.contains("sdf_thing")){
        for (const auto& thing : sdf_model_json.at("sdf_thing").items()) {
            SdfThing sdfThing;
            parseSdfThing(thing.value(), sdfThing);
            sdf_model.sdf_thing[thing.key()] = sdfThing;
        }
    }

    // Parse the sdfObjects
    else if (sdf_model_json.contains("sdf_object")) {
        for (const auto& object : sdf_model_json.at("sdf_object").items()) {
            SdfObject sdfObject;
            parseSdfObject(object.value(), sdfObject);
            sdf_model.sdf_object[object.key()] = sdfObject;
        }
    }
    // As described in Section 3.4 [https://datatracker.ietf.org/doc/draft-ietf-asdf-sdf/], SDF grants the possibility
    // to have sdf_property, sdf_action or sdf_event as a top level affordance. But as these are meant to be used as
    // re-usable definitions for usage via sdf_ref, these are meaningless for the converter, hence they're ignored.

    // If neither sdf_thing nor sdf_object are present, the model is empty and thus invalid
    else {
        return -1;
    }

    return 0;
}

/*
 * Parse a sdf-mapping from json into a SdfMapping object.
 */
int parseSdfMapping(json& sdf_mapping_json, SdfMapping& sdf_mapping)
{
    // Parse the information block
    if (sdf_mapping_json.contains("info")) {
        parseInfoBlock(sdf_mapping_json.at("info"), sdf_mapping.information_block);
    }

    // Parse the namespace block
    if (sdf_mapping_json.contains("namespace")) {
        if (sdf_mapping_json.contains("default_namespace")) {
            parseNamespaceBlock(sdf_mapping_json.at("namespace"), sdf_mapping.namespace_block);
            sdf_mapping_json.at("default_namespace").get_to(sdf_mapping.namespace_block.default_namespace);
        }
    }

    // Parse the map section
    if (sdf_mapping_json.contains("map")) {
        for (const auto& reference : sdf_mapping_json.at("map").items()) {
            for (const auto& field : reference.value().items()) {
                sdf_mapping.map[reference.key()].insert({field.key(), field.value()});
            }
        }
    }

    return 0;
}

/*
 * Serialize common qualities into the json format.
 */
int serializeCommonQualities(const CommonQuality& commonQuality, json& common_quality_json)
{
    if (!commonQuality.description.empty())
        common_quality_json["description"] = commonQuality.description;

    if (!commonQuality.label.empty())
        common_quality_json["label"] = commonQuality.label;

    if (!commonQuality.comment.empty())
        common_quality_json["comment"] = commonQuality.comment;

    if (!commonQuality.sdf_ref.empty())
        common_quality_json["sdf_ref"] = commonQuality.sdf_ref;

    if (!commonQuality.sdf_required.empty())
        common_quality_json["sdf_required"] = commonQuality.sdf_required;

    return 0;
}

/*
 * Function prototype used for recursive calls.
 */
int serializeDataQualities(const DataQuality& dataQuality, json& data_quality_json);

/*
 * Serialize jso items into the json format.
 */
int serializeJsoItemType(const JsoItem& jsoItem, json& jso_item_type_json)
{
    if (!jsoItem.sdf_ref.empty())
        jso_item_type_json["sdf_ref"] = jsoItem.sdf_ref;

    if (!jsoItem.description.empty())
        jso_item_type_json["description"] = jsoItem.description;

    if (!jsoItem.comment.empty())
        jso_item_type_json["comment"] = jsoItem.comment;

    if (!jsoItem.type.empty())
        jso_item_type_json["type"] = jsoItem.type;

    // Iterate through all fields and serialize them individually
    if (!jsoItem.sdf_choice.empty()){
        json sdf_choice_map_json;
        for (const auto& sdf_choice_map : jsoItem.sdf_choice) {
            json sdf_choice_json = json({});
            serializeDataQualities(sdf_choice_map.second, sdf_choice_json);
            sdf_choice_map_json[sdf_choice_map.first] = sdf_choice_json;
        }
        jso_item_type_json["sdf_choice"] = sdf_choice_map_json;
    }

    if (!jsoItem.enum_.empty())
        jso_item_type_json["enum"] = jsoItem.enum_;

    if (jsoItem.minimum.has_value())
        jso_item_type_json["minimum"] = jsoItem.minimum.value();

    if (jsoItem.maximum.has_value())
        jso_item_type_json["maximum"] = jsoItem.maximum.value();

    if (!jsoItem.format.empty())
        jso_item_type_json["format"] = jsoItem.format;

    if (jsoItem.min_length.has_value())
        jso_item_type_json["min_length"] = jsoItem.min_length.value();

    if (jsoItem.max_length.has_value())
        jso_item_type_json["max_length"] = jsoItem.max_length.value();

    if (!jsoItem.properties.empty()) {
        json sdf_properties_json;
        for (const auto& data_quality : jsoItem.properties) {
            json current_jso_item_type_json;
            serializeDataQualities(data_quality.second, current_jso_item_type_json);
            sdf_properties_json[data_quality.first] = current_jso_item_type_json;
        }
        jso_item_type_json["properties"] = sdf_properties_json;
    }

    if (!jsoItem.required.empty())
        jso_item_type_json["required"] = jsoItem.required;

    return 0;
}

/*
 * Serialize data qualities into the json format.
 */
int serializeDataQualities(const DataQuality& dataQuality, json& data_quality_json)
{
    // Serialize common qualities
    serializeCommonQualities(dataQuality, data_quality_json);

    // Serialize the remaining fields
    if (!dataQuality.type.empty())
        data_quality_json["type"] = dataQuality.type;

    // Iterate through all fields and serialize them individually
    if (!dataQuality.sdf_choice.empty()){
        json sdf_choice_map_json;
        for (const auto& sdf_choice_map : dataQuality.sdf_choice) {
            json sdf_choice_json = json({});
            serializeDataQualities(sdf_choice_map.second, sdf_choice_json);
            sdf_choice_map_json[sdf_choice_map.first] = sdf_choice_json;
        }
        data_quality_json["sdf_choice"] = sdf_choice_map_json;
    }

    if (!dataQuality.enum_.empty())
        data_quality_json["enum"] = dataQuality.enum_;

    if (dataQuality.const_.has_value()) {
        // TODO: Can the object type have a default value?
        if (dataQuality.type == "number") {
            data_quality_json["const"] = std::get<double>(dataQuality.default_.value());
        } else if (dataQuality.type == "string") {
            data_quality_json["const"] = std::get<std::string>(dataQuality.default_.value());
        } else if (dataQuality.type == "boolean") {
            data_quality_json["const"] = std::get<bool>(dataQuality.default_.value());
        } else if (dataQuality.type == "integer") {
            data_quality_json["const"] = std::get<uint64_t>(dataQuality.default_.value());
            // TODO: Maybe set this to either int64 or uint64, check json documentation
        } else if (dataQuality.type == "array") {
            //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        } else if (dataQuality.type == "object") {
        //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        }
    }
    //    data_quality_json["const"] = dataQuality.const_;

    // Depending on the type, use a different function read the type
    //TODO: Maybe write a generic helper to cast this
    if (dataQuality.default_.has_value()) {
        // TODO: Can the object type have a default value?
        if (dataQuality.type == "number") {
            data_quality_json["default"] = std::get<double>(dataQuality.default_.value());
        } else if (dataQuality.type == "string") {
            data_quality_json["default"] = std::get<std::string>(dataQuality.default_.value());
        } else if (dataQuality.type == "boolean") {
            data_quality_json["default"] = std::get<bool>(dataQuality.default_.value());
        } else if (dataQuality.type == "integer") {
            // TODO: Maybe set this to either int64 or uint64, check json documentation
            data_quality_json["default"] = std::get<uint64_t>(dataQuality.default_.value());
        } else if (dataQuality.type == "array") {
            //data_quality_json["default"] = std::get<double>(dataQuality.default_.value());
        } else if (dataQuality.type == "object") {
            //data_qualities_json.at("default").get_to(dataQuality.default_->array);
        }
    }

    if (dataQuality.minimum.has_value()) {
        if (dataQuality.type == "number") {
            data_quality_json["minimum"] = std::get<double>(dataQuality.minimum.value());
        } else if (dataQuality.type == "integer") {
            if (std::holds_alternative<int64_t>(dataQuality.minimum.value()))
                data_quality_json["minimum"] = std::get<int64_t >(dataQuality.minimum.value());
            if (std::holds_alternative<uint64_t>(dataQuality.minimum.value()))
                data_quality_json["minimum"] = std::get<uint64_t >(dataQuality.minimum.value());
        }
    }

    if (dataQuality.maximum.has_value()) {
        if (dataQuality.type == "number") {
            data_quality_json["maximum"] = std::get<double>(dataQuality.maximum.value());
        } else if (dataQuality.type == "integer") {
            if (std::holds_alternative<int64_t>(dataQuality.maximum.value()))
                data_quality_json["maximum"] = std::get<int64_t >(dataQuality.maximum.value());
            if (std::holds_alternative<uint64_t>(dataQuality.maximum.value()))
                data_quality_json["maximum"] = std::get<uint64_t >(dataQuality.maximum.value());
        }
    }

    if (dataQuality.exclusive_minimum.has_value()) {
        if (dataQuality.type == "number") {
            data_quality_json["exclusive_minimum"] = std::get<double>(dataQuality.exclusive_minimum.value());
        } else if (dataQuality.type == "integer") {
            if (std::holds_alternative<int64_t>(dataQuality.exclusive_minimum.value()))
                data_quality_json["exclusive_minimum"] = std::get<int64_t >(dataQuality.exclusive_minimum.value());
            if (std::holds_alternative<uint64_t>(dataQuality.exclusive_minimum.value()))
                data_quality_json["exclusive_minimum"] = std::get<uint64_t >(dataQuality.exclusive_minimum.value());
        }
    }

    if (dataQuality.exclusive_maximum.has_value()) {
        if (dataQuality.type == "number") {
            data_quality_json["exclusive_maximum"] = std::get<double>(dataQuality.exclusive_maximum.value());
        } else if (dataQuality.type == "integer") {
            if (std::holds_alternative<int64_t>(dataQuality.exclusive_maximum.value()))
                data_quality_json["exclusive_maximum"] = std::get<int64_t >(dataQuality.exclusive_maximum.value());
            if (std::holds_alternative<uint64_t>(dataQuality.exclusive_maximum.value()))
                data_quality_json["exclusive_maximum"] = std::get<uint64_t >(dataQuality.exclusive_maximum.value());
        }
    }

    if (dataQuality.multiple_of.has_value()) {
        if (dataQuality.type == "number") {
            data_quality_json["multiple_of"] = std::get<double>(dataQuality.multiple_of.value());
        } else if (dataQuality.type == "integer") {
            if (std::holds_alternative<int64_t>(dataQuality.multiple_of.value()))
                data_quality_json["multiple_of"] = std::get<int64_t >(dataQuality.multiple_of.value());
            if (std::holds_alternative<uint64_t>(dataQuality.multiple_of.value()))
                data_quality_json["multiple_of"] = std::get<uint64_t >(dataQuality.multiple_of.value());
        }
    }

    if (dataQuality.min_length.has_value())
        data_quality_json["min_length"] = dataQuality.min_length.value();

    if (dataQuality.max_length.has_value())
        data_quality_json["max_length"] = dataQuality.max_length.value();

    if (!dataQuality.pattern.empty())
        data_quality_json["pattern"] = dataQuality.pattern;

    if (!dataQuality.format.empty())
        data_quality_json["format"] = dataQuality.format;

    if (dataQuality.min_items.has_value())
        data_quality_json["min_items"] = dataQuality.min_items.value();

    if (dataQuality.max_items.has_value())
        data_quality_json["max_items"] = dataQuality.max_items.value();

    if (dataQuality.unique_items.has_value())
        data_quality_json["unique_items"] = dataQuality.unique_items.value();

    if (dataQuality.items.has_value()) {
        json jso_item_json;
        serializeJsoItemType(dataQuality.items.value(), jso_item_json);
        data_quality_json["items"] = jso_item_json;
    }

    if (!dataQuality.properties.empty()) {
        json sdf_properties_json;
        for (const auto& data_quality : dataQuality.properties) {
            json current_data_quality_json;
            serializeDataQualities(data_quality.second, current_data_quality_json);
            sdf_properties_json[data_quality.first] = current_data_quality_json;
        }
        data_quality_json["properties"] = sdf_properties_json;
    }

    if (!dataQuality.required.empty())
        data_quality_json["required"] = dataQuality.required;

    if (!dataQuality.unit.empty())
        data_quality_json["unit"] = dataQuality.unit;

    if (dataQuality.nullable.has_value())
        data_quality_json["nullable"] = dataQuality.nullable.value();

    if (!dataQuality.sdf_type.empty())
        data_quality_json["sdf_type"] = dataQuality.sdf_type;

    if (!dataQuality.content_format.empty())
        data_quality_json["content_format"] = dataQuality.content_format;

    return 0;
}

/*
 * Serialize sdf_data into the json format.
 */
int serializeSdfData(const SdfData& sdfData, json& sdf_data_json)
{
    // Iterate through all elements and serialize them individually
    for (const auto& data_quality_map : sdfData){
        json data_quality_json;
        serializeDataQualities(data_quality_map.second, data_quality_json);
        sdf_data_json[data_quality_map.first] = data_quality_json;
    }

    return 0;
}

/*
 * Serialize a sdf_event into the json format.
 */
int serializeSdfEvent(const SdfEvent& sdfEvent, json& sdf_event_json)
{
    // Serialize common qualities
    serializeCommonQualities(sdfEvent,sdf_event_json);

    // Serialize the output data
    json sdf_output_data_json;
    serializeDataQualities(sdfEvent.sdf_output_data, sdf_output_data_json);
    sdf_event_json["sdf_output_data"] = sdf_output_data_json;

    // Serialize the sdf_data elements
    json sdf_data_json;
    serializeSdfData(sdfEvent.sdf_data, sdf_data_json);
    sdf_event_json["sdf_data"] = sdf_data_json;

    return 0;
}

/*
 * Serialize a sdf_action into the json format.
 */
int serializeSdfAction(const SdfAction& sdfAction, json& sdf_action_json)
{
    // Serialize common qualities
    serializeCommonQualities(sdfAction, sdf_action_json);

    // Serialize the input data
    if (sdfAction.sdf_input_data.has_value()) {
        json sdf_input_data_json;
        serializeDataQualities(sdfAction.sdf_input_data.value(), sdf_input_data_json);
        sdf_action_json["sdf_input_data"] = sdf_input_data_json;
    }

    // Serialize the output data
    if (sdfAction.sdf_output_data.has_value()) {
        json sdf_output_data_json;
        serializeDataQualities(sdfAction.sdf_output_data.value(), sdf_output_data_json);
        sdf_action_json["sdf_output_data"] = sdf_output_data_json;
    }

    // Serialize the sdf_data elements
    if (sdfAction.sdf_data.has_value()) {
        json sdf_data_json;
        serializeSdfData(sdfAction.sdf_data.value(), sdf_data_json);
        sdf_action_json["sdf_data"] = sdf_data_json;
    }

    return 0;
}

/*
 * Serialize a sdf_property into the json format.
 */
int serializeSdfProperty(const SdfProperty& sdfProperty, json& sdf_property_json)
{
    // Serialize data qualities
    serializeDataQualities(sdfProperty, sdf_property_json);

    // Serialize the remaining fields
    if (sdfProperty.readable.has_value())
        sdf_property_json["readable"] = sdfProperty.readable.value();

    if (sdfProperty.writable.has_value())
        sdf_property_json["writable"] = sdfProperty.writable.value();

    if (sdfProperty.observable.has_value())
        sdf_property_json["observable"] = sdfProperty.observable.value();

    return 0;
}

/*
 * Serialize a sdf_object into the json format
 */
int serializeSdfObject(const SdfObject& sdfObject, json& sdf_object_json)
{
    // Serialize the common qualities
    serializeCommonQualities(sdfObject, sdf_object_json);

    // Serialize the sdfProperties
    if (!sdfObject.sdf_property.empty()) {
        json sdf_property_map_json;
        for (const auto& sdf_property_map: sdfObject.sdf_property) {
            json sdf_property_json;
            serializeSdfProperty(sdf_property_map.second, sdf_property_json);
            sdf_property_map_json[sdf_property_map.first] = sdf_property_json;
        }
        sdf_object_json["sdf_property"] = sdf_property_map_json;
    }

    // Serialize the sdfActions
    if (!sdfObject.sdf_action.empty()) {
        json sdf_action_map_json;
        for (const auto& sdf_action_map: sdfObject.sdf_action) {
            json sdf_action_json;
            serializeSdfAction(sdf_action_map.second, sdf_action_json);
            sdf_action_map_json[sdf_action_map.first] = sdf_action_json;
        }
        sdf_object_json["sdf_action"] = sdf_action_map_json;
    }

    // Serialize the sdfEvents
    if (!sdfObject.sdf_event.empty()) {
        json sdf_event_map_json;
        for (const auto& sdf_event_map: sdfObject.sdf_event) {
            json sdf_event_json;
            serializeSdfEvent(sdf_event_map.second, sdf_event_json);
            sdf_event_map_json[sdf_event_map.first] = sdf_event_json;
        }
        sdf_object_json["sdf_event"] = sdf_event_map_json;
    }

    // Serialize the sdf_data
    if (!sdfObject.sdf_data.empty()) {
        json sdf_data_json;
        serializeSdfData(sdfObject.sdf_data, sdf_data_json);
        sdf_object_json["sdf_data"] = sdf_data_json;
    }

    // Serialize the remaining fields
    if (sdfObject.min_items.has_value())
        sdf_object_json["min_items"] = sdfObject.min_items.value();

    if (sdfObject.max_items.has_value())
        sdf_object_json["max_items"] = sdfObject.max_items.value();

    return 0;
}

/*
 * Serialize a sdf_thing into the json format.
 */
int serializeSdfThing(const SdfThing& sdfThing, json& sdf_thing_json)
{
    // Serialize the common qualities
    serializeCommonQualities(sdfThing, sdf_thing_json);

    // sdf_thing

    // Serialize the sdfObjects
    if (!sdfThing.sdf_object.empty()) {
        json sdf_object_map_json;
        for (const auto& sdf_object_map: sdfThing.sdf_object) {
            json sdf_object_json;
            serializeSdfObject(sdf_object_map.second, sdf_object_json);
            sdf_object_map_json[sdf_object_map.first] = sdf_object_json;
        }
        sdf_thing_json["sdf_object"] = sdf_object_map_json;
    }

    // Serialize the sdfProperties
    if (!sdfThing.sdf_property.empty()) {
        json sdf_property_map_json;
        for (const auto& sdf_property_map: sdfThing.sdf_property) {
            json sdf_property_json;
            serializeSdfProperty(sdf_property_map.second, sdf_property_json);
            sdf_property_map_json[sdf_property_map.first] = sdf_property_json;
        }
        sdf_thing_json["sdf_property"] = sdf_property_map_json;
    }

    // Serialize the sdfActions
    if (!sdfThing.sdf_action.empty()) {
        json sdf_action_map_json;
        for (const auto& sdf_action_map: sdfThing.sdf_action) {
            json sdf_action_json;
            serializeSdfAction(sdf_action_map.second, sdf_action_json);
            sdf_action_map_json[sdf_action_map.first] = sdf_action_json;
        }
        sdf_thing_json["sdf_action"] = sdf_action_map_json;
    }

    // Serialize the sdfEvents
    if (!sdfThing.sdf_event.empty()) {
        json sdf_event_map_json;
        for (const auto& sdf_event_map: sdfThing.sdf_event) {
            json sdf_event_json;
            serializeSdfEvent(sdf_event_map.second, sdf_event_json);
            sdf_event_map_json[sdf_event_map.first] = sdf_event_json;
        }
        sdf_thing_json["sdf_event"] = sdf_event_map_json;
    }

    // Serialize the sdf_data
    if (!sdfThing.sdf_data.empty()) {
        json sdf_data_json;
        serializeSdfData(sdfThing.sdf_data, sdf_data_json);
        sdf_thing_json["sdf_data"] = sdf_data_json;
    }

    // Serialize the remaining fields
    if (sdfThing.min_items.has_value())
        sdf_thing_json["min_items"] = sdfThing.min_items.value();

    if (sdfThing.max_items.has_value())
        sdf_thing_json["max_items"] = sdfThing.max_items.value();

    return 0;
}

/*
 * Serialize a namespace block into the json format.
 */
int serializeNamespaceBlock(const NamespaceBlock& namespaceBlock, json& namespace_block_json)
{
    if (!namespaceBlock.namespaces.empty())
        namespace_block_json["namespace"] = namespaceBlock.namespaces;

    if (!namespaceBlock.default_namespace.empty())
        namespace_block_json["default_namespace"] = namespaceBlock.default_namespace;

    return 0;
}

/*
 * Serialize an information block into the json format.
 */
int serializeInfoBlock(const InformationBlock& infoBlock, json& info_block_json)
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

    if (!infoBlock.comment.empty())
        info_block_json["comment"] = infoBlock.comment;

    return 0;
}

/*
 * Serialize a sdf-model into the json format.
 */
int serializeSdfModel(const SdfModel& sdf_model, json& sdf_model_json)
{
    // Serialize the information block
    json info_block_json;
    serializeInfoBlock(sdf_model.information_block, info_block_json);
    if (!info_block_json.is_null())
        sdf_model_json["info"] = info_block_json;

    // Serialize the namespace block
    serializeNamespaceBlock(sdf_model.namespace_block, sdf_model_json);

    // Serialize the sdf_thing
    if (!sdf_model.sdf_thing.empty()){
        json sdf_thing_json;
        for (const auto& thing : sdf_model.sdf_thing) {
            json current_sdf_thing_json;
            serializeSdfThing(thing.second, current_sdf_thing_json);
            sdf_thing_json[thing.first] = current_sdf_thing_json;
        }
        sdf_model_json["sdf_thing"] = sdf_thing_json;
    }

    // Serialize the sdf_object
    else if (!sdf_model.sdf_object.empty()){
        json sdf_object_json;
        for (const auto& object : sdf_model.sdf_object) {
            json current_sdf_object_json;
            serializeSdfObject(object.second, current_sdf_object_json);
            sdf_object_json[object.first] = current_sdf_object_json;
        }
        sdf_model_json["sdf_object"] = sdf_object_json;
    }
    else {
        // If neither sdf_thing nor sdf_object are present, the models definition block is empty and thus invalid
        return -1;
    }

    return 0;
}

/*
 * Serialize a sdf-mapping back into the json format.
 */
int serializeSdfMapping(const SdfMapping& sdf_mapping, json& sdf_mapping_json)
{
    // Serialize the information block
    json info_block_json;
    serializeInfoBlock(sdf_mapping.information_block, info_block_json);
    if (!info_block_json.is_null())
        sdf_mapping_json["info"] = info_block_json;

    // Serialize the namespace block
    serializeNamespaceBlock(sdf_mapping.namespace_block, sdf_mapping_json);

    // Serialize the mapping section
    sdf_mapping_json["map"] = sdf_mapping.map;

    return 0;
}

} // namespace sdf
