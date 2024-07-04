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
#include <regex>
#include <functional>
#include <utility>
#include <limits>
#include <set>
#include "mapping.h"
#include "matter.h"
#include "sdf.h"

//! Map containing the elements of the sdf mapping
//! This map is used to resolve the elements outsourced into the map
json reference_map;

//! Set containing the supported features
//! Contains their short code for usage with conformance's
std::set<std::string> supported_features;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
std::list<std::string> sdf_required_list;

//! Function to escape JSON Pointer according to RFC 6901
std::string EscapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find('~', pos)) != std::string::npos) {
        result.replace(pos, 1, "~0");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find('/', pos)) != std::string::npos) {
        result.replace(pos, 1, "~1");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find(' ', pos)) != std::string::npos) {
        result.replace(pos, 1, "%20");
        pos += 3;
    }
    return result;
}

class ReferenceTreeNode {
public:
    std::string name;
    std::map<std::string, sdf::MappingValue> attributes;
    ReferenceTreeNode* parent;
    std::vector<ReferenceTreeNode*> children;

    ReferenceTreeNode(std::string name) : name(std::move(name)), attributes(), parent(nullptr) {}

    void AddChild(ReferenceTreeNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    void AddAttribute(const std::string& key, sdf::MappingValue value) {
        attributes[key] = std::move(value);
    }

    std::string GeneratePointer() {
        std::string path;
        ReferenceTreeNode* current = this;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
    }
};

class ReferenceTree {
public:
    ReferenceTreeNode* root;

    ReferenceTree() {
        root = new ReferenceTreeNode("#");
    }

    std::map<std::string, std::map<std::string, sdf::MappingValue>> GenerateMapping(ReferenceTreeNode* node) {
        std::map<std::string, std::map<std::string, sdf::MappingValue>> map;
        ReferenceTreeNode* current = node;
        for (const auto& child : current->children) {
            if (!child->attributes.empty()) {
                map[GeneratePointer(child)] = child->attributes;
            }
            map.merge(GenerateMapping(child));
        }
        return map;
    }

    std::string GeneratePointer(ReferenceTreeNode* node) {
        std::string path;
        ReferenceTreeNode* current = node;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
    }
};

//! This is a global pointer to the quality name current node
//! This is designed to point at the top level sdf element like
//! for example the `sdfThing` node, not a specific sdfThing
ReferenceTreeNode* current_quality_name_node = nullptr;

//! This is a global pointer to the given name current node
//! This is designed to point at the given name of an element
//! for example the `OnOff` node, not a top level sdf element
ReferenceTreeNode* current_given_name_node = nullptr;

//! Function to unescape JSON Pointer according to RFC 6901
std::string UnescapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find("~1", pos)) != std::string::npos) {
        result.replace(pos, 2, "/");
        pos += 1;
    }
    pos = 0;
    while ((pos = result.find("~0", pos)) != std::string::npos) {
        result.replace(pos, 2, "~");
        pos += 1;
    }
    pos = 0;
    while ((pos = result.find("%20", pos)) != std::string::npos) {
        result.replace(pos, 3, " ");
        pos += 1;
    }
    return result;
}

bool CheckForRequired(const std::string& json_pointer)
{

    //if (sdf_required_list.)
    return true;
}

//! Generically typed for now as the mapping will later contain different types
//! @brief Imports given key value combination from the SDF Mapping file
//! @param name Name of the target field.
template <typename T> void ImportFromMapping(const std::string& json_pointer, const std::string& field, T& input)
{
    if (reference_map.contains(json_pointer)) {
        if (reference_map.at(json_pointer).contains(field)) {
            reference_map.at(json_pointer).at(field).get_to(input);
        }
    }
}

std::optional<matter::Access> ImportAccessFromMapping(const std::string& json_pointer)
{
    json access_json;
    ImportFromMapping(json_pointer, "access", access_json);
    if (access_json.is_null())
        return std::nullopt;
    matter::Access access;
    if (access_json.contains("read"))
        access_json.at("read").get_to(access.read);
    if (access_json.contains("write"))
        access_json.at("write").get_to(access.write);
    if (access_json.contains("fabricScoped"))
        access_json.at("fabricScoped").get_to(access.fabric_scoped);
    if (access_json.contains("fabricSensitive"))
        access_json.at("fabricSensitive").get_to(access.fabric_sensitive);
    if (access_json.contains("readPrivilege"))
        access_json.at("readPrivilege").get_to(access.read_privilege);
    if (access_json.contains("writePrivilege"))
        access_json.at("writePrivilege").get_to(access.write_privilege);
    if (access_json.contains("invokePrivilege"))
        access_json.at("invokePrivilege").get_to(access.invoke_privilege);
    if (access_json.contains("timed"))
        access_json.at("timed").get_to(access.timed);
    return access;
}

std::optional<matter::OtherQuality> ImportOtherQualityFromMapping(const std::string& json_pointer)
{
    //TODO: Fix non_volatile to its actual field
    json other_quality_json;
    ImportFromMapping(json_pointer, "quality", other_quality_json);
    if (other_quality_json.is_null())
        return std::nullopt;
    matter::OtherQuality other_quality;
    if (other_quality_json.contains("nullable"))
        other_quality_json.at("nullable").get_to(other_quality.nullable);
    if (other_quality_json.contains("nonVolatile"))
        other_quality_json.at("nonVolatile").get_to(other_quality.non_volatile);
    if (other_quality_json.contains("fixed"))
        other_quality_json.at("fixed").get_to(other_quality.fixed);
    if (other_quality_json.contains("scene"))
        other_quality_json.at("scene").get_to(other_quality.scene);
    if (other_quality_json.contains("reportable"))
        other_quality_json.at("reportable").get_to(other_quality.reportable);
    if (other_quality_json.contains("changeOmitted"))
        other_quality_json.at("changeOmitted").get_to(other_quality.change_omitted);
    if (other_quality_json.contains("singleton"))
        other_quality_json.at("singleton").get_to(other_quality.singleton);
    if (other_quality_json.contains("diagnostics"))
        other_quality_json.at("diagnostics").get_to(other_quality.diagnostics);
    if (other_quality_json.contains("largeMessage"))
        other_quality_json.at("largeMessage").get_to(other_quality.large_message);
    if (other_quality_json.contains("quieterReporting"))
        other_quality_json.at("quieterReporting").get_to(other_quality.quieter_reporting);
    return other_quality;
}

matter::Conformance ImportConformanceFromMapping();

//! Generates a Matter access based on information of the provided sdfProperty
matter::Access GenerateMatterAccess(sdf::SdfProperty)
{
    matter::Access access;
    return access;
}

//! @brief Generates a Matter conformance.
//! The resulting conformance depends on the following factors:
//! - required quality for the object type
//! - being part of a sdfRequired quality
//! If the referred element mentioned in either of these factors,
//! a mandatory conformance will be created.
//! Otherwise a optional conformance will be created.
matter::Conformance GenerateMatterConformance(std::string& json_pointer)
{
    matter::Conformance conformance;
    /*
    if (ImportFromMapping(json_pointer, "conformance"))
    else if (CheckForRequired(""))
        conformance.mandatory = true;
    else
        conformance.optional = true;
    */
    return conformance;
}

//! Generates a Matter constraint with the information given by the data qualities
matter::Constraint GenerateMatterConstraint(const sdf::DataQuality& dataQuality)
{
    matter::Constraint constraint;
    //constraint.value = dataQuality.default_;
    if (dataQuality.type == "number" or dataQuality.type == "integer") {
        if (dataQuality.minimum.has_value())
            constraint.min = dataQuality.minimum.value();
        if (dataQuality.maximum.has_value())
            constraint.max = dataQuality.maximum.value();
        // exclusive_minimum
        // exclusive_maximum
        // multiple_of
    } else if (dataQuality.type == "string") {
        if (dataQuality.min_length.has_value())
            constraint.min = dataQuality.min_length.value();
        if (dataQuality.max_length.has_value())
            constraint.max = dataQuality.max_length.value();
        // pattern
        // format
    } else if (dataQuality.type == "array") {
        if (dataQuality.min_items.has_value())
            constraint.min = dataQuality.min_items.value();
        if (dataQuality.max_items.has_value())
            constraint.max = dataQuality.max_items.value();
        // unique_items
        // items -> Translate these into entry constraints
    } else if (dataQuality.type == "object") {
        // Currently does not seem like object contains usefull information for constraints
        // properties
        // required
    }
    return constraint;
}

std::string MapIntegerType(const sdf::DataQuality& data_quality)
{
    if (data_quality.minimum.has_value()) {}
    if (data_quality.maximum.has_value()) {}
    return "";
}

//! Determine a Matter type from the information's of a given data quality
std::string MapSdfDataType(const sdf::DataQuality& data_quality)
{
    std::string result;
    if (data_quality.type == "number") {
        result = "double";
    } else if (data_quality.type == "string") {
        result = "string";
    } else if (data_quality.type == "boolean") {
        result = "bool";
    } else if (data_quality.type == "integer") {

    } else if (data_quality.type == "array") {

    } else if (data_quality.type == "object") {
        result = "struct";
    } else if (data_quality.sdf_type == "byte-string") {
        result = "octstr";
    } else if (data_quality.sdf_type == "unix-time") {

    }
    return result;
}

//! Maps a data quality onto a data field.
matter::DataField MapSdfData(sdf::DataQuality& data_quality)
{
    matter::DataField data_field;
    // data_field.id
    data_field.name = data_quality.label;
    //data_field.conformance = GenerateMatterConformance();
    // data_field.access
    data_field.summary = data_quality.description;
    // data_field.type =
    data_field.constraint = GenerateMatterConstraint(data_quality);
    // data_field.quality;
    // data_field.default_ = data_quality.default_;
    return data_field;
}

//! Maps a sdfEvent onto a Matter event
matter::Event MapSdfEvent(const std::pair<std::string, sdf::SdfEvent>& sdf_event_pair)
{
    matter::Event event;
    auto* sdf_event_reference = new ReferenceTreeNode(sdf_event_pair.first);
    current_quality_name_node->AddChild(sdf_event_reference);
    //TODO: Event needs an ID, this needs to be set here
    event.name = sdf_event_pair.second.label;
    event.summary = sdf_event_pair.second.description;
    //sdf_required TODO: Collect these and set the conformance for the corresponding element to mandatory
    //sdf_output_data TODO: How do we map sdf_output_data to the Event Fields?
    for (auto elem : sdf_event_pair.second.sdf_data) {
        MapSdfData(elem.second);
    }
    return event;
}

//! Maps either a sdfInputData or sdfOutputData element onto a Matter data field
matter::DataField MapSdfInputOutputData(const sdf::DataQuality& data_quality)
{
    matter::DataField data_field;
    data_field.summary = data_quality.description;
    data_field.name = data_quality.label;
    //comment
    //sdf_required
    data_field.type = MapSdfDataType(data_quality);
    //sdf_choice
    //enum
    //const
    //default
    data_field.constraint = GenerateMatterConstraint(data_quality);
    //exclusive_minimum
    //exclusive_maximum
    //multiple_of
    //pattern
    //format
    //unique_items
    //items
    //unit
    //nullable
    //sdf_type
    //content_format

    return data_field;
}

//! Maps a sdfAction onto a Matter client and optionally on a server command
std::pair<matter::Command, std::optional<matter::Command>> MapSdfAction(const std::pair<std::string, sdf::SdfAction>& sdf_action_pair)
{
    matter::Command client_command;
    auto* sdf_action_reference = new ReferenceTreeNode(sdf_action_pair.first);
    current_quality_name_node->AddChild(sdf_action_reference);

    ImportFromMapping(sdf_action_reference->GeneratePointer(), "id", client_command.id);
    client_command.name = sdf_action_pair.second.label;
    // conformance
    client_command.access = ImportAccessFromMapping(sdf_action_reference->GeneratePointer());
    client_command.summary = sdf_action_pair.second.description;
    // default
    client_command.direction = "commandToServer";
    std::optional<matter::Command> optional_server_command;
    // Check if the sdfAction has output data qualities
    if (sdf_action_pair.second.sdf_output_data.has_value()) {
        //TODO: According to spec, the id should be different
        // Initially, we copy the contents of the client command
        matter::Command server_command = client_command;
        // If object is used as a type, the elements of the object have to be mapped individually
        if (sdf_action_pair.second.sdf_output_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                server_command.command_fields.push_back(field);
            }
        }
        else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_output_data.value());
            server_command.command_fields.push_back(field);
        }
        //required
        optional_server_command = server_command;
    } else {
        client_command.response = "N";
    }
    // TODO: Consider the status code here

    // Map the sdf_input_data Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdf_action_pair.second.sdf_input_data.has_value()) {
        // If object is used as a type, the elements of the object have to be mapped individually
        if (sdf_action_pair.second.sdf_input_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                client_command.command_fields.push_back(field);
            }
            //required
        } else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_input_data.value());
            client_command.command_fields.push_back(field);
        }
    }

    return {client_command, optional_server_command};
}

//! Maps a sdfProperty onto a Matter attribute
matter::Attribute MapSdfProperty(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair)
{
    matter::Attribute attribute;
    auto* sdf_property_reference = new ReferenceTreeNode(sdf_property_pair.first);
    current_quality_name_node->AddChild(sdf_property_reference);

    ImportFromMapping(sdf_property_reference->GeneratePointer(), "id", attribute.id);
    attribute.name = sdf_property_pair.second.label;
    // sdf_property.comment
    // sdf_property.sdf_required
    // conformance
    attribute.access = ImportAccessFromMapping(sdf_property_reference->GeneratePointer());
    attribute.access->write = sdf_property_pair.second.writable;
    attribute.access->read = sdf_property_pair.second.readable;
    // sdf_property.observable
    attribute.summary = sdf_property_pair.second.description;
    attribute.type = MapSdfDataType(sdf_property_pair.second);
    //attribute.default_ = sdf_property.default_;
    attribute.quality = ImportOtherQualityFromMapping(sdf_property_reference->GeneratePointer());
    //attribute.quality.nullable = sdf_property.nullable;
    //attribute.quality.fixed = !sdf_property.const_.empty();

    return attribute;
}

//! Maps a sdfObject onto a Matter cluster
matter::Cluster MapSdfObject(const std::pair<std::string, sdf::SdfObject>& sdf_object_pair)
{
    matter::Cluster cluster;
    auto* sdf_object_reference = new ReferenceTreeNode(sdf_object_pair.first);
    current_quality_name_node->AddChild(sdf_object_reference);

    ImportFromMapping(sdf_object_reference->GeneratePointer(), "id", cluster.id);
    cluster.name = sdf_object_pair.second.label;
    // conformance
    cluster.summary = sdf_object_pair.second.description;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "revision", cluster.revision);
    json revision_history_json;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "revisionHistory", revision_history_json);
    for (const auto& item : revision_history_json) {
        u_int8_t revision;
        item.at("revision").get_to(revision);
        std::string summary;
        item.at("summary").get_to(summary);
        cluster.revision_history[revision] = summary;
    }
    // classification

    // Iterate through all sdfProperties and parse them individually
    auto* sdf_property_reference = new ReferenceTreeNode("sdfProperty");
    sdf_object_reference->AddChild(sdf_property_reference);
    current_quality_name_node = sdf_property_reference;
    for (const auto& sdf_property_pair : sdf_object_pair.second.sdf_property) {
        cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
    }

    // Iterate through all sdfActions and parse them individually
    auto* sdf_action_reference = new ReferenceTreeNode("sdfAction");
    sdf_object_reference->AddChild(sdf_action_reference);
    current_quality_name_node = sdf_action_reference;
    for (const auto& sdf_action_pair : sdf_object_pair.second.sdf_action) {
        std::pair<matter::Command, std::optional<matter::Command>> command_pair = MapSdfAction(sdf_action_pair);
        cluster.client_commands.push_back(command_pair.first);
        if (command_pair.second.has_value())
            cluster.server_commands[command_pair.second.value().name] = command_pair.second.value();
    }

    // Iterate through all sdfEvents and parse them individually
    auto* sdf_event_reference = new ReferenceTreeNode("sdfEvent");
    sdf_object_reference->AddChild(sdf_event_reference);
    current_quality_name_node = sdf_event_reference;
    for (const auto& sdf_event_pair : sdf_object_pair.second.sdf_event) {
        cluster.events.push_back(MapSdfEvent(sdf_event_pair));
    }

    return cluster;
}

//! Maps a sdfThing onto a Matter device type
matter::Device MapSdfThing(const std::pair<std::string, sdf::SdfThing>& sdf_thing_pair)
{
    matter::Device device;
    // Add the current sdf_thing to the reference tree
    auto* sdf_thing_reference = new ReferenceTreeNode(sdf_thing_pair.first);
    current_quality_name_node->AddChild(sdf_thing_reference);
    // Import the ID from the mapping
    ImportFromMapping(sdf_thing_reference->GeneratePointer(), "id", device.id);
    device.name = sdf_thing_pair.second.label;
    device.summary = sdf_thing_pair.second.description;
    // revision
    // revision_history

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdf_object_pair : sdf_thing_pair.second.sdf_object) {
        current_quality_name_node = new ReferenceTreeNode("sdfObject");
        sdf_thing_reference->AddChild(current_quality_name_node);
        device.clusters.push_back(MapSdfObject(sdf_object_pair));
    }

    return device;
}

//! Creates Matter optional_device as well as cluster definitions from a given SDF Model and SDF Mapping
int MapSdfToMatter(const sdf::SdfModel& sdf_model,
                   const sdf::SdfMapping& sdf_mapping,
                   std::optional<matter::Device>& optional_device,
                   std::list<matter::Cluster>& cluster_list)
{
    // Make the mapping a global variable
    if (!sdf_mapping.map.empty()) {
        reference_map = sdf_mapping.map;
    }

    // Initialize a reference tree used to resolve json references
    ReferenceTree reference_tree;

    if (!sdf_model.sdf_thing.empty()){
        current_quality_name_node = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(current_quality_name_node);
        for (const auto& sdf_thing_pair : sdf_model.sdf_thing) {
            optional_device = MapSdfThing(sdf_thing_pair);
            if (optional_device.has_value())
                cluster_list = optional_device.value().clusters;
        }
    } else if (!sdf_model.sdf_object.empty()){
        // Make sure, that optional_device is empty, as there is no sdfThing present
        optional_device.reset();
        current_quality_name_node = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(current_quality_name_node);
        for (const auto& sdf_object_pair : sdf_model.sdf_object) {
            matter::Cluster cluster = MapSdfObject(sdf_object_pair);
            cluster_list.push_back(cluster);
        }
    }

    return 0;
}

//! Maps information of the given other quality onto a sdfProperty object
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::SdfProperty& sdf_property)
{
    json quality_json;
    if (other_quality.nullable.has_value())
        sdf_property.nullable = other_quality.nullable.value();
    if (other_quality.non_volatile.has_value())
        quality_json["nonVolatile"] = other_quality.non_volatile.value();
    if (other_quality.fixed.has_value())
        quality_json["fixed"] = other_quality.fixed.value();
    if (other_quality.scene.has_value())
        quality_json["scene"] = other_quality.scene.value();
    if (other_quality.reportable.has_value())
        sdf_property.observable = other_quality.reportable.value();
    if (other_quality.change_omitted.has_value())
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    if (other_quality.singleton.has_value())
        quality_json["singleton"] = other_quality.singleton.value();
    if (other_quality.diagnostics.has_value())
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    if (other_quality.large_message.has_value())
        quality_json["largeMessage"] = other_quality.large_message.value();
    if (other_quality.quieter_reporting.has_value())
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    if (!quality_json.is_null())
        current_given_name_node->AddAttribute("quality", quality_json);
}

//! Maps information of the given other quality onto a data quality object
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::DataQuality& data_quality)
{
    // TODO: Change nonVolatile to its actual value from the xml
    json quality_json;
    if (other_quality.nullable.has_value())
        data_quality.nullable = other_quality.nullable.value();
    if (other_quality.non_volatile.has_value())
        quality_json["nonVolatile"] = other_quality.non_volatile.value();
    if (other_quality.fixed.has_value())
        quality_json["fixed"] = other_quality.fixed.value();
    if (other_quality.scene.has_value())
        quality_json["scene"] = other_quality.scene.value();
    if (other_quality.reportable.has_value())
        quality_json["reportable"] = other_quality.reportable.value();
    if (other_quality.change_omitted.has_value())
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    if (other_quality.singleton.has_value())
        quality_json["singleton"] = other_quality.singleton.value();
    if (other_quality.diagnostics.has_value())
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    if (other_quality.large_message.has_value())
        quality_json["largeMessage"] = other_quality.large_message.value();
    if (other_quality.quieter_reporting.has_value())
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    if (!quality_json.is_null())
        current_given_name_node->AddAttribute("quality", quality_json);
}

std::pair<std::string, sdf::DataQuality> MapMatterBitfield(const std::pair<std::string, std::list<matter::Bitfield>>& bitmap_pair)
{
    sdf::DataQuality data_quality;
    data_quality.type = "object";
    for (const auto& bitfield : bitmap_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        sdf_choice_data_quality.const_ = bitfield.bit;
        sdf_choice_data_quality.description = bitfield.summary;
        // conformance
        data_quality.sdf_choice[bitfield.name] = sdf_choice_data_quality;
    }
    //required
    return {bitmap_pair.first, data_quality};
}

std::pair<std::string, sdf::DataQuality> MapMatterEnum(const std::pair<std::string, std::list<matter::Item>>& enum_pair)
{
    sdf::DataQuality data_quality;
    data_quality.type = "object";
    for (const auto& item : enum_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        sdf_choice_data_quality.const_ = item.value;
        sdf_choice_data_quality.description = item.summary;
        // conformance
        data_quality.sdf_choice[item.name] = sdf_choice_data_quality;
    }
    //required
    return {enum_pair.first, data_quality};
}

//! Generates data qualities based on the given matter type
void MapMatterType(const std::string& matter_type, sdf::DataQuality& data_quality)
{
    // Base Matter data types
    // Boolean data type
    if (matter_type == "bool") {
        data_quality.type = "boolean";
    }
    // Bitmap data type
    else if (matter_type.substr(0, 3) == "map") {
        data_quality.type = "array";
        data_quality.unique_items = true;
        if (matter_type.substr(4) == "8") {
            data_quality.max_items = 8;
        }
        else if (matter_type.substr(4) == "16") {
            data_quality.max_items = 16;
        }
        else if (matter_type.substr(4) == "32") {
            data_quality.max_items = 32;
        }
        else if (matter_type.substr(4) == "64") {
            data_quality.max_items = 64;
        }
    }
    // Unsigned integer data type
    else if (matter_type.substr(0, 4) == "uint") {
        // TODO: These boundaries change if the corresponding value is nullable
        data_quality.type = "integer";
        data_quality.minimum = 0;
        if (matter_type.substr(4) == "8") {
            data_quality.maximum = MATTER_U_INT_8_MAX;
        } else if (matter_type.substr(4) == "16") {
            data_quality.maximum = MATTER_U_INT_16_MAX;
        } else if (matter_type.substr(4) == "24") {
            data_quality.maximum = MATTER_U_INT_24_MAX;
        } else if (matter_type.substr(4) == "32") {
            data_quality.maximum = MATTER_U_INT_32_MAX;
        } else if (matter_type.substr(4) == "40") {
            data_quality.maximum = MATTER_U_INT_40_MAX;
        } else if (matter_type.substr(4) == "48") {
            data_quality.maximum = MATTER_U_INT_48_MAX;
        } else if (matter_type.substr(4) == "56") {
            data_quality.maximum = MATTER_U_INT_56_MAX;
        } else if (matter_type.substr(4) == "64") {
            data_quality.maximum = std::numeric_limits<uint64_t>::min();
        }
    }
    // Signed integer data type
    else if (matter_type.substr(0, 3) == "int") {
        data_quality.type = "integer";
        if (matter_type.substr(3) == "8") {
            data_quality.minimum = MATTER_INT_8_MIN;
            data_quality.maximum = MATTER_INT_8_MAX;
        } else if (matter_type.substr(3) == "16") {
            data_quality.minimum = MATTER_INT_16_MIN;
            data_quality.maximum = MATTER_INT_16_MAX;
        } else if (matter_type.substr(3) == "24") {
            data_quality.minimum = MATTER_INT_24_MIN;
            data_quality.maximum = MATTER_INT_24_MAX;
        } else if (matter_type.substr(3) == "32") {
            data_quality.minimum = MATTER_INT_32_MIN;
            data_quality.maximum = MATTER_INT_32_MAX;
        } else if (matter_type.substr(3) == "40") {
            data_quality.minimum = MATTER_INT_40_MIN;
            data_quality.maximum = MATTER_INT_40_MAX;
        } else if (matter_type.substr(3) == "48") {
            data_quality.minimum = MATTER_INT_48_MIN;
            data_quality.maximum = MATTER_INT_48_MAX;
        } else if (matter_type.substr(3) == "56") {
            data_quality.minimum = MATTER_INT_56_MIN;
            data_quality.maximum = MATTER_INT_56_MAX;
        } else if (matter_type.substr(3) == "64") {
            data_quality.minimum = std::numeric_limits<int64_t>::min();
            data_quality.maximum = std::numeric_limits<int64_t>::max();
        }
    }
    // Single precision floating point data type
    else if (matter_type == "single") {
        data_quality.type = "number";
        data_quality.minimum = std::numeric_limits<float>::min();
        data_quality.minimum = std::numeric_limits<float>::max();
    }
    // Double precision floating point data type
    else if (matter_type == "double") {
        data_quality.type = "number";
        data_quality.minimum = std::numeric_limits<double>::min();
        data_quality.minimum = std::numeric_limits<double>::max();
    }
    // Octet string data type
    else if (matter_type == "octstr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // List data type
    else if (matter_type == "list") {
        data_quality.type = "array";
    }
    // Struct data type
    else if (matter_type == "struct") {
        data_quality.type = "object";
    }
    // Derived Matter data types
    // Percent data type
    // Base: uint8
    else if (matter_type == "percent") {
        data_quality.type = "integer";
        data_quality.unit = "%";
        data_quality.minimum = 0;
        data_quality.maximum = 100;
    }
    // Percent 100th data type
    // Base: uint16
    else if (matter_type == "percent100ths") {
        data_quality.type = "integer";
        data_quality.unit = "%";
        data_quality.minimum = 0;
        data_quality.maximum = 10000;
    }
    // Time of day data type
    // Base: struct
    else if (matter_type == "tod") {
        data_quality.type = "object";
    }
    // Data data type
    // Base: struct
    else if (matter_type == "date") {
        data_quality.type = "object";
    }
    // Epoch time in microseconds data type
    // Base: uint64
    else if (matter_type == "epoch-us") {}
    // Epoch time in seconds data type
    // Base: uint32
    else if (matter_type == "epoch-s") {}
    // UTC time data type
    // Base: uint32
    // DEPRECATED
    else if (matter_type == "utc") {}
    // POSIX time in milliseconds
    // Base: uint64
    else if (matter_type == "posix-ms") {}
    // System time in microseconds
    // Base: uint64
    else if (matter_type == "systime-us") {}
    // System time in milliseconds
    // Base: uint64
    else if (matter_type == "systime-ms") {}
    // Elapsed time in seconds data type
    // Base: uint32
    else if (matter_type == "elapsed-s") {}
    // Temperature data type
    // Base: int16
    else if (matter_type == "temperature") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.minimum = -27315;
        data_quality.maximum = 32767;
    }
    // Power data type
    // Base: int64
    else if (matter_type == "power-mW") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mW";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Amperage data type
    // Base: int64
    else if (matter_type == "amperage-mA") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mA";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Voltage data type
    // Base: int64
    else if (matter_type == "voltage-mW") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mV";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Energy data type
    // Base: int64
    else if (matter_type == "energy-mWh") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mWh";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // 8-bit enumeration data type
    // Base: uint8
    else if (matter_type == "enum8") {}
    // 16-bit enumeration data type
    // Base: uint16
    else if (matter_type == "enum16") {}
    // Priority data type
    // Base: enum8
    else if (matter_type == "priority") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
        sdf::DataQuality debug_priority;
        //debug_priority.label = "DEBUG";
        debug_priority.const_ = 0;
        debug_priority.description = "Information for engineering debugging/troubleshooting";
        sdf::DataQuality info_priority;
        //info_priority.label = "INFO";
        info_priority.const_ = 1;
        info_priority.description = "Information that either drives customer facing features or provides insights into device functions that are used to drive analytics use cases";
        sdf::DataQuality critical_priority;
        //critical_priority.label = "CRITICAL";
        critical_priority.const_ = 2;
        critical_priority.description = "Information or notification that impacts safety, a critical function, or ongoing reliable operation of the node or application supported on an endpoint";
        data_quality.sdf_choice["DEBUG"] = debug_priority;
        data_quality.sdf_choice["INFO"] = info_priority;
        data_quality.sdf_choice["CRITICAL"] = critical_priority;
    }
    // Status code data type
    // Base: enum8
    else if (matter_type == "status") {
        data_quality.label = matter_type;
    }
    // Group id data type
    // Base: uint16
    else if (matter_type == "group-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Endpoint number data type
    // Base: uint16
    else if (matter_type == "endpoint-no") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Vendor id data type
    // Base: uint16
    else if (matter_type == "vendor-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Device type id data type
    // Base: uint32
    else if (matter_type == "devtype-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Fabric id data type
    // Base: uint64
    else if (matter_type == "fabric-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // Fabric index data type
    // Base: uint8
    else if (matter_type == "fabric-idx") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
    }
    // Cluster id data type
    // Base: uint32
    else if (matter_type == "cluster-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Attribute id data type
    // Base: uint32
    else if (matter_type == "attrib-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Field id data type
    // Base: uint32
    else if (matter_type == "field-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Event id data type
    // Base: uint32
    else if (matter_type == "event-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Command id data type
    // Base: uint32
    else if (matter_type == "command-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Action id data type
    // Base: uint8
    else if (matter_type == "action-id") {data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;}
    // Transaction id data type
    // Base: uint32
    else if (matter_type == "trans-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Node id data type
    // Base: uint64
    else if (matter_type == "node-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // IEEE address data type
    // Base: uint64
    // DEPRECATED
    else if (matter_type == "EUI64") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();}
    // Entry index data type
    // Base: uint16
    else if (matter_type == "entry-idx") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Data version data type
    // Base: uint32
    else if (matter_type == "data-ver") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Event number data type
    // Base: uint64
    else if (matter_type == "event-no") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // Character string data type
    // Base: octstr
    else if (matter_type == "string") {
        data_quality.type = "string";
    }
    // IP address data type
    // Base: ocstr
    else if (matter_type == "ipadr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // IPv4 address data type
    // Base: octstr
    else if (matter_type == "ipv4adr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
        data_quality.min_length = 8;
        data_quality.max_length = 8;
    }
    // IPv6 address data type
    // Base: octstr
    else if (matter_type == "ipv6adr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
        data_quality.min_length = 32;
        data_quality.max_length = 32;
    }
    // IPv6 prefix data type
    // Base: octstr
    else if (matter_type == "ipv6pre") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // Hardware address data type
    // Base: octstr
    else if (matter_type == "hwadr") {
        data_quality.type = "string";
        data_quality.min_length = 12;
        data_quality.max_length = 16;
    }
    // Semantic tag data type
    // Base: struct
    else if (matter_type == "semtag") {
        data_quality.type = "object";
        sdf::DataQuality mfg_code;
        mfg_code.type = "integer";
        mfg_code.minimum = 0;
        mfg_code.maximum = MATTER_U_INT_16_MAX;
        mfg_code.nullable = true;
        // mfg_code.default_ = null;
        sdf::DataQuality namespace_id;
        namespace_id.type = "integer";
        namespace_id.minimum = 0;
        namespace_id.maximum = MATTER_U_INT_16_MAX;
        sdf::DataQuality tag;
        tag.type = "integer";
        tag.minimum = 0;
        tag.maximum = MATTER_U_INT_16_MAX;
        sdf::DataQuality label;
        label.type = "string";
        label.max_length = 64;
        label.nullable = true;
        //label.default_ = null;
        data_quality.required = {"MfgCode", "NamespaceID", "Tag", "Label"};
    }
    // Namespace data type
    // Base: enum8
    else if (matter_type == "namespace") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Tag data type
    // Base: enum8
    else if (matter_type == "tag") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Otherwise, the type is a custom type defined in the data type section
    else {
        data_quality.label = matter_type;
        //data_quality.sdf_ref
        std::cout << "Found: " << matter_type << std::endl;
    }
}

//! Matter Constraint -> Data Quality
void MapMatterConstraint(const matter::Constraint& constraint, sdf::DataQuality& data_quality)
{
    // We ignore the "desc" constraint type as its dependent on the implementation of the cluster
    if (data_quality.type == "number" or data_quality.type == "integer") {
        if (constraint.value.has_value()) {}
            //data_quality.const_ = constraint.value;
        if (constraint.from.has_value())
            data_quality.minimum = constraint.from.value();
        if (constraint.to.has_value())
            data_quality.maximum = constraint.to.value();
        if (constraint.min.has_value())
            data_quality.minimum = constraint.min.value();
        if (constraint.max.has_value())
            data_quality.maximum = constraint.max.value();
    }
    else if (data_quality.type == "string") {
        if (constraint.value.has_value()) {}
        //data_quality.const_ = constraint.value;
        if (constraint.from.has_value()) {}
        if (constraint.to.has_value()) {}
        if (constraint.min.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.min_length = std::get<uint64_t>(constraint.min.value());
        }

        if (constraint.max.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.max.value()))
                data_quality.max_length = std::get<uint64_t>(constraint.max.value());
        }
    }
    else if (data_quality.type == "array") {
        if (constraint.value.has_value()) {}
        //data_quality.const_ = constraint.value;
        if (constraint.from.has_value()) {}
        if (constraint.to.has_value()) {}
        if (constraint.min.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.min_items = std::get<uint64_t>(constraint.min.value());
        }
        if (constraint.max.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.max_items = std::get<uint64_t>(constraint.min.value());
        }
    }
    else if (data_quality.type == "object") {}
}

//! Matter Access Type -> SDF Mapping
//! This function is used standalone to move all qualities to the SDF Mapping
void MapMatterAccess(const matter::Access& access)
{
    json access_json;
    if (access.read.has_value())
        access_json["read"] = access.read.value();
    if (access.write.has_value())
        access_json["write"] = access.write.value();
    if (access.fabric_scoped.has_value())
        access_json["fabricScoped"] = access.fabric_scoped.value();
    if (access.fabric_sensitive.has_value())
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    if (!access.read_privilege.empty())
        access_json["readPrivilege"] = access.read_privilege;
    if (!access.write_privilege.empty())
        access_json["writePrivilege"] = access.write_privilege;
    if (!access.invoke_privilege.empty())
        access_json["invokePrivilege"] = access.invoke_privilege;
    if (access.timed.has_value())
        access_json["timed"] = access.timed.value();

    current_given_name_node->AddAttribute("access", access_json);
}

//! Matter Access Type
//! This function is used in combination with a sdfProperty
void MapMatterAccess(const matter::Access& access, sdf::SdfProperty& sdf_property)
{
    json access_json;
    if (access.read.has_value())
        sdf_property.readable = access.read.value();
    if (access.write.has_value())
        sdf_property.writable = access.write.value();
    if (access.fabric_scoped.has_value())
        access_json["fabricScoped"] = access.fabric_scoped.value();
    if (access.fabric_sensitive.has_value())
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    if (!access.read_privilege.empty())
        access_json["readPrivilege"] = access.read_privilege;
    if (!access.write_privilege.empty())
        access_json["writePrivilege"] = access.write_privilege;
    if (!access.invoke_privilege.empty())
        access_json["invokePrivilege"] = access.invoke_privilege;
    if (access.timed.has_value())
        access_json["timed"] = access.timed.value();

    current_given_name_node->AddAttribute("access", access_json);
}

bool EvaluateConformanceCondition(const json& condition)
{
    if (condition.is_null())
        return true;
    else if (condition.contains("andTerm")) {
        // Return true, if all the contained expressions evaluate to true
        // Returns false otherwise
        for (auto& item : condition.at("andTerm")) {
            if (!EvaluateConformanceCondition(item))
                return false;
        }
        return true;
    }
    else if (condition.contains("orTerm")) {
        // Returns true, if any one of the contained expressions evaluate to true
        // Returns false otherwise
        for (auto& item : condition.at("orTerm")) {
            if (EvaluateConformanceCondition(item))
                return true;
        }
        return false;
    }
    else if (condition.contains("xorTerm")) {
        // Returns true, if just one of the contained expressions evaluates to true
        // Returns false otherwise
        bool evaluated_one = false;
        for (auto& item : condition.at("xorTerm")) {
            if (EvaluateConformanceCondition(item)) {
                if (!evaluated_one)
                    evaluated_one = true;
                else
                    return true;
            }
            return evaluated_one;
        }
    }
    else if (condition.contains("notTerm")) {
        return !EvaluateConformanceCondition(condition.at("notTerm"));
    }
    else if (condition.contains("feature")) {
        std::cout << "Reached" << condition.at("feature") << std::endl;
        if (supported_features.find(condition.at("feature").at("name")) != supported_features.end()) {
            std::cout << "Feature" << condition.at("feature") << "supported" << std::endl;
            return true;
        }
    }
    else if (condition.contains("condition")) {
        std::cout << "Reached" << condition.at("condition") << std::endl;
        // TODO: Check if the condition is satisfied
        return true;
    }
    else if (condition.contains("attribute")) {
        std::cout << "Reached" << condition.at("attribute") << std::endl;
        // TODO: Check if the attribute exists
        return true;
    }
    return false;
}

/**
 * @brief Adds element to sdf_required depending on the conformance.
 *
 * This function adds the current element to sdf_required it is set to mandatory via its conformance.
 *
 * @param conformance The conformance to check.
 * @param current_node The reference tree node of the current element.
 * @return 0 on success, negative on failure.
 */
bool MapMatterConformance(const matter::Conformance& conformance) {
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value() and EvaluateConformanceCondition(conformance.condition)) {
            sdf_required_list.push_back(current_given_name_node->GeneratePointer());
        }
    }

    if (!conformance.condition.is_null()) {
        if (conformance.mandatory.has_value())
            current_given_name_node->AddAttribute("mandatoryConform", conformance.condition);
        else if (conformance.optional.has_value())
            current_given_name_node->AddAttribute("optionalConform", conformance.condition);
        else if (conformance.provisional.has_value())
            current_given_name_node->AddAttribute("provisionalConform", conformance.condition);
        else if (conformance.deprecated.has_value())
            current_given_name_node->AddAttribute("deprecatedConform", conformance.condition);
        else if (conformance.disallowed.has_value())
            current_given_name_node->AddAttribute("disallowConform", conformance.condition);
    } else {
        return true;
    }

    return EvaluateConformanceCondition(conformance.condition);
}

sdf::DataQuality MapMatterDataField(const std::list<matter::DataField>& data_field_list)
{
    sdf::DataQuality data_quality;
    //id
    //conformance
    //default
    if (data_field_list.empty()) {}
    else if (data_field_list.size() <= 1) {
        data_quality.label = data_field_list.front().name;
        if (data_field_list.front().access.has_value())
            MapMatterAccess(data_field_list.front().access.value());
        data_quality.description = data_field_list.front().summary;
        MapMatterType(data_field_list.front().type, data_quality);
        if (data_field_list.front().quality.has_value())
            MapOtherQuality(data_field_list.front().quality.value(), data_quality);
        if (data_field_list.front().constraint.has_value())
            MapMatterConstraint(data_field_list.front().constraint.value(), data_quality);
    } else {
        data_quality.type = "object";
        for (const auto& field : data_field_list) {
            sdf::DataQuality data_quality_properties;
            data_quality_properties.label = field.name;
            if (field.access.has_value())
                MapMatterAccess(field.access.value());
            data_quality_properties.description = field.summary;
            MapMatterType(field.type, data_quality_properties);
            if (field.quality.has_value())
                MapOtherQuality(field.quality.value(), data_quality_properties);
            if (field.constraint.has_value())
                MapMatterConstraint(field.constraint.value(), data_quality);
            data_quality.properties[field.name] = data_quality_properties;
        }
        //required
    }
    return data_quality;
}

sdf::SdfEvent MapMatterEvent(const matter::Event& event)
{
    sdf::SdfEvent sdf_event;
    // Append the event node to the tree
    auto* event_reference = new ReferenceTreeNode(event.name);
    current_quality_name_node->AddChild(event_reference);
    current_given_name_node = event_reference;
    // Export the id to the mapping
    event_reference->AddAttribute("id", (uint64_t) event.id);

    sdf_event.label = event.name;
    if (event.conformance.has_value())
        MapMatterConformance(event.conformance.value());
    if (event.access.has_value())
        MapMatterAccess(event.access.value());
    sdf_event.description = event.summary;

    // Export priority to the mapping
    event_reference->AddAttribute("priority", event.priority);
    sdf_event.sdf_output_data = MapMatterDataField(event.data);
    // TODO: Event itself should probably not have these qualities
    //if (event.quality.has_value())
    //    MapOtherQuality(event.quality.value(), sdf_event.sdf_output_data.value());

    return sdf_event;
}

sdf::SdfAction MapMatterCommand(const matter::Command& client_command, const std::map<std::string, matter::Command>& server_commands)
{
    sdf::SdfAction sdf_action;
    // Append the client_command node to the tree
    auto* command_reference = new ReferenceTreeNode(client_command.name);
    current_quality_name_node->AddChild(command_reference);
    current_given_name_node = command_reference;
    // If the command does not have a response
    if (client_command.response == "N") {}
    // If the client_command only returns a simple status
    else if (client_command.response == "Y") {
        sdf::DataQuality sdf_output_data;
        sdf_output_data.type = "integer";
        sdf_output_data.minimum = MATTER_INT_16_MIN;
        sdf_output_data.maximum = MATTER_U_INT_16_MAX;
    }
    // Otherwise, the client client_command has a reference to a server client_command
    else {
        sdf_action.sdf_output_data = MapMatterDataField(server_commands.at(client_command.response).command_fields);
    }

    // Export the id to the mapping
    command_reference->AddAttribute("id", (uint64_t) client_command.id);
    sdf_action.label = client_command.name;
    if (client_command.conformance.has_value())
        MapMatterConformance(client_command.conformance.value());
    if (client_command.access.has_value())
        MapMatterAccess(client_command.access.value());
    sdf_action.description = client_command.summary;
    // client_command.default_
    if (!client_command.command_fields.empty())
        sdf_action.sdf_input_data = MapMatterDataField(client_command.command_fields);

    return sdf_action;
}

sdf::SdfProperty MapMatterAttribute(const matter::Attribute& attribute)
{
    sdf::SdfProperty sdf_property;
    // Append the attribute node to the tree
    auto* attribute_reference = new ReferenceTreeNode(attribute.name);
    current_quality_name_node->AddChild(attribute_reference);
    current_given_name_node = attribute_reference;

    // Export the id to the mapping
    attribute_reference->AddAttribute("id", (uint64_t) attribute.id);
    sdf_property.label = attribute.name;

    if (attribute.conformance.has_value())
        MapMatterConformance(attribute.conformance.value());

    if (attribute.access.has_value())
        MapMatterAccess(attribute.access.value(), sdf_property);

    sdf_property.description = attribute.summary;

    // Map the Matter type onto data qualities
    MapMatterType(attribute.type, sdf_property);

    if (attribute.constraint.has_value())
        MapMatterConstraint(attribute.constraint.value(), sdf_property);

    if (attribute.quality.has_value())
        MapOtherQuality(attribute.quality.value(), sdf_property);

    // sdf_property.default_ = attribute.default_;

    return sdf_property;
}

void MapFeatureMap(const std::list<matter::Feature>& feature_map)
{
    // Evaluate the features while also exporting them to the mapping
    json feature_map_json;
    for (const auto& feature : feature_map) {
        json feature_json;
        bool condition;
        feature_json["bit"] = feature.bit;
        feature_json["code"] = feature.code;
        feature_json["name"] = feature.name;
        feature_json["summary"] = feature.summary;
        if (feature.conformance.has_value()) {
            condition = MapMatterConformance(feature.conformance.value());
            if (feature.conformance.value().mandatory.has_value() and condition) {
                supported_features.insert(feature.code);
                std::cout << "Supported Feature: " << feature.code << std::endl;
            }
        }
        feature_map_json.push_back(feature_json);
    }
    current_quality_name_node->AddAttribute("features", feature_map_json);
}

void MapClusterClassification(const matter::ClusterClassification& cluster_classification)
{
    json cluster_classification_json;
    cluster_classification_json["hierarchy"] = cluster_classification.hierarchy;
    cluster_classification_json["role"] = cluster_classification.role;
    cluster_classification_json["picsCode"] = cluster_classification.picsCode;
    cluster_classification_json["scope"] = cluster_classification.scope;
    cluster_classification_json["baseCluster"] = cluster_classification.base_cluster;
    cluster_classification_json["primaryTransaction"] = cluster_classification.primary_transaction;
    current_quality_name_node->AddAttribute("classification", cluster_classification_json);
}

sdf::SdfObject MapMatterCluster(const matter::Cluster& cluster)
{
    sdf::SdfObject sdf_object;
    ReferenceTreeNode* cluster_reference;
    // When combined with a device type definition, we have to differentiate between server and client clusters, as they
    // differentiate in the way they are translated. Furthermore, as a device type can have the same cluster as a server
    // as well as a client cluster at the same time, we have to add an appendix to their given name, as they would
    // otherwise override each other. If side is missing, that indicates, that the cluster is processed without a
    // device type definition.
    if (cluster.side == "client") {
        cluster_reference = new ReferenceTreeNode(cluster.name + "_Client");
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
        cluster_reference->AddAttribute("side", cluster.side);
    } else if (cluster.side == "server"){
        cluster_reference = new ReferenceTreeNode(cluster.name + "_Server");
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
        cluster_reference->AddAttribute("side", cluster.side);
    } else {
        cluster_reference = new ReferenceTreeNode(cluster.name);
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
    }

    cluster_reference->AddAttribute("id", (uint64_t) cluster.id);
    sdf_object.label = cluster.name;
    if (cluster.conformance.has_value())
        MapMatterConformance(cluster.conformance.value());

    sdf_object.description = cluster.summary;
    cluster_reference->AddAttribute("revision", cluster.revision);
    // Export the revision history to the mapping
    json revision_history_json;
    for (const auto& revision : cluster.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json.push_back(revision_json);
    }
    cluster_reference->AddAttribute("revisionHistory", revision_history_json);

    if (cluster.classification.has_value())
        MapClusterClassification(cluster.classification.value());

    MapFeatureMap(cluster.feature_map);

    // Iterate through the attributes and map them
    auto* sdf_property_node = new ReferenceTreeNode("sdfProperty");
    cluster_reference->AddChild(sdf_property_node);
    current_quality_name_node = sdf_property_node;
    for (const auto& attribute : cluster.attributes){
        sdf::SdfProperty sdf_property = MapMatterAttribute(attribute);
        sdf_object.sdf_property.insert({attribute.name, sdf_property});
    }

    // Iterate through the commands and map them
    auto* sdf_action_node = new ReferenceTreeNode("sdfAction");
    cluster_reference->AddChild(sdf_action_node);
    current_quality_name_node = sdf_action_node;
    for (const auto& command : cluster.client_commands){
        sdf::SdfAction sdf_action = MapMatterCommand(command, cluster.server_commands);
        sdf_object.sdf_action.insert({command.name, sdf_action});
    }

    // Iterate through the events and map them
    auto* sdf_event_node = new ReferenceTreeNode("sdfEvent");
    cluster_reference->AddChild(sdf_event_node);
    current_quality_name_node = sdf_event_node;
    for (const auto& event : cluster.events){
        sdf::SdfEvent sdf_event =  MapMatterEvent(event);
        sdf_object.sdf_event.insert({event.name, sdf_event});
    }

    for (const auto& enum_pair : cluster.enums) {
        sdf_object.sdf_data.insert(MapMatterEnum(enum_pair));
    }

    for (const auto& bitmap_pair : cluster.bitmaps) {
        sdf_object.sdf_data.insert(MapMatterBitfield(bitmap_pair));
    }

    sdf_object.sdf_required = sdf_required_list;

    return sdf_object;
}

//! Generate a SDF-Model or SDF-Mapping information block based on information of either
//! a Matter device or a Matter cluster
sdf::InformationBlock GenerateInformationBlock(const std::variant<matter::Device, matter::Cluster>& input)
{
    sdf::InformationBlock information_block;
    if (std::holds_alternative<matter::Device>(input)) {
        information_block.title = std::get<matter::Device>(input).name;
        information_block.description = std::get<matter::Device>(input).summary;
    } else if (std::holds_alternative<matter::Cluster>(input)) {
        information_block.title = std::get<matter::Cluster>(input).name;
        information_block.description = std::get<matter::Cluster>(input).summary;
    }
    return information_block;
}

void MapDeviceClassification(const matter::DeviceClassification& device_classification)
{
    json device_classification_json;
    device_classification_json["superset"] = device_classification.superset;
    device_classification_json["class"] = device_classification.class_;
    device_classification_json["scope"] = device_classification.scope;
    current_given_name_node->AddAttribute("classification", device_classification_json);
}

sdf::SdfThing MapMatterDevice(const matter::Device& device)
{
    sdf::SdfThing sdf_thing;
    // Append a new sdf_object node to the tree
    auto* device_reference = new ReferenceTreeNode(device.name);
    current_quality_name_node->AddChild(device_reference);
    current_given_name_node = device_reference;

    device_reference->AddAttribute("id", (uint64_t) device.id);
    if (device.classification.has_value())
        MapDeviceClassification(device.classification.value());
    if (device.conformance.has_value())
        MapMatterConformance(device.conformance.value());
    // Export the revision history to the mapping
    device_reference->AddAttribute("revision", device.revision);
    json revision_history_json;
    for (const auto& revision : device.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json.push_back(revision_json);
    }
    device_reference->AddAttribute("revisionHistory", revision_history_json);

    sdf_thing.label = device.name;
    sdf_thing.description = device.summary;

    // Iterate through cluster definitions for the device
    auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
    device_reference->AddChild(sdf_object_reference);
    current_quality_name_node = sdf_object_reference;
    for (const auto& cluster : device.clusters){
        sdf::SdfObject sdf_object = MapMatterCluster(cluster);
        // Clear the sdfRequired list as it would result in duplicates
        sdf_object.sdf_required.clear();
        if (cluster.side == "client")
            sdf_thing.sdf_object.insert({cluster.name + "_Client", sdf_object});
        else if (cluster.side == "server")
            sdf_thing.sdf_object.insert({cluster.name + "_Server", sdf_object});
        else {
            std::cout << "Something went wrong" << std::endl;
        }
        current_quality_name_node = sdf_object_reference;
        // Clear the list of supported features after every run
        supported_features.clear();
    }
    sdf_thing.sdf_required = sdf_required_list;

    return sdf_thing;
}

//! Function used to merge device and cluster specifications together
void MergeDeviceCluster(matter::Device& device, const std::list<matter::Cluster>& cluster_list)
{
    for (auto& device_cluster : device.clusters) {
        for (const auto& cluster: cluster_list) {
            if (device_cluster.id == cluster.id) {
                matter::Cluster temp_cluster = cluster;
                // Overwrite the conformance for the cluster
                temp_cluster.conformance = device_cluster.conformance;
                // Set the side of the cluster
                temp_cluster.side = device_cluster.side;
                // Overwrite the feature conformance's
                for (auto &device_feature: device_cluster.feature_map) {
                    for (auto &cluster_feature: temp_cluster.feature_map) {
                        if (device_feature.name == cluster_feature.name) {
                            cluster_feature.conformance = device_feature.conformance;
                        }
                    }
                }

                // Overwrite certain attributes
                for (auto &device_attribute: device_cluster.attributes) {
                    for (auto &cluster_attribute: temp_cluster.attributes) {
                        if (device_attribute.name == cluster_attribute.name) {

                        }
                    }
                }
                // Overwrite certain commands
                for (auto &client_command: device_cluster.client_commands) {

                }
                // Overwrite certain commands
                for (auto &server_command: device_cluster.server_commands) {

                }
                // Overwrite certain events
                for (auto &device_event: device_cluster.events) {
                    for (auto &cluster_event: temp_cluster.events) {
                        if (device_event.name == cluster_event.name) {

                        }
                    }
                }
                device_cluster = temp_cluster;
            }
        }
    }
}

int MapMatterToSdf(const std::optional<matter::Device>& optional_device,
                   const std::list<matter::Cluster>& cluster_list,
                   sdf::SdfModel& sdf_model, sdf::SdfMapping& sdf_mapping)
{
    ReferenceTree reference_tree;
    if (optional_device.has_value()) {
        auto* sdf_thing_reference = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(sdf_thing_reference);
        current_quality_name_node = sdf_thing_reference;
        matter::Device device = optional_device.value();
        sdf_model.information_block = GenerateInformationBlock(device);
        sdf_mapping.information_block = GenerateInformationBlock(device);
        MergeDeviceCluster(device, cluster_list);
        sdf::SdfThing sdf_thing = MapMatterDevice(device);
        sdf_model.sdf_thing.insert({sdf_thing.label, sdf_thing});
    } else {
        auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(sdf_object_reference);
        current_quality_name_node = sdf_object_reference;
        for (const auto& cluster : cluster_list) {
            sdf_model.information_block = GenerateInformationBlock(cluster);
            sdf_mapping.information_block = GenerateInformationBlock(cluster);
            sdf::SdfObject sdf_object = MapMatterCluster(cluster);
            sdf_model.sdf_object.insert({sdf_object.label, sdf_object});
            current_quality_name_node = sdf_object_reference;
        }
    }

    sdf_mapping.map = reference_tree.GenerateMapping(reference_tree.root);

    return 0;
}
