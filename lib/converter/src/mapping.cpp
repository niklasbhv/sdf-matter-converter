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
#include "mapping.h"
#include "matter.h"
#include "sdf.h"

//! For debug purposes, prints a visual representation of the tree
struct simple_walker: pugi::xml_tree_walker
{
    bool for_each(pugi::xml_node& node) override
    {
        for (int i = 0; i < depth(); ++i) std::cout << "--> "; // indentation

        std::cout << node.name() << std::endl;

        return true; // continue traversal
    }
};

int resolve_mappings(clusterType& cluster)
{
    return 0;
}

int save_to_mapping(const char* name, const std::string& value)
{
    //deviceTree.append_attribute(name) = value.c_str();
    return 0;
}

template <typename T>

int get_from_mapping(const char* name, T& value)
{
    return 0;
}

//! sdfEvent -> Matter event
int map_sdf_event(sdfEventType& sdfEvent, eventType& event)
{
    event.name = sdfEvent.label;
    event.description = sdfEvent.description;
    // access
    // field
    // code
    // side
    // priority
    return 0;
}

//! sdfAction -> Matter command
int map_sdf_action(sdfActionType& sdfAction, commandType& command)
{
    command.name = sdfAction.label;
    command.description = sdfAction.description;
    //TODO: This needs further mapping as these need to be split up
    return 0;
}

//! sdfProperty -> Matter attribute
int map_sdf_property(sdfPropertyType& sdfProperty, attributeType& attribute)
{
    attribute.name = sdfProperty.label;
    attribute.description = sdfProperty.description;
    // access
    // code
    attribute.default_ = sdfProperty.default_;
    // define
    // introducedIn
    // length
    // manufacturerCode
    //TODO: Does this match?
    if (sdfProperty.maximum.has_value())
        attribute.max = sdfProperty.maximum.value();
    if (sdfProperty.minimum.has_value())
        attribute.min = sdfProperty.minimum.value();
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    // optional
    // side
    attribute.type = sdfProperty.type; //TODO: Definitely needs mapping
    // reportable
    // array
    attribute.isNullable = sdfProperty.nullable;
    //TODO: This will default to false, check documentation for the actual default
    if (sdfProperty.readable.has_value())
        attribute.readable = sdfProperty.readable.value();
    if (sdfProperty.writable.has_value())
        attribute.writable = sdfProperty.writable.value();
    return 0;
}

//! sdfObject -> Matter cluster
int map_sdf_object(sdfObjectType& sdfObject, clusterType& cluster)
{
    // Common qualities
    cluster.name = sdfObject.label;
    cluster.description = sdfObject.description;
    save_to_mapping("domain", cluster.domain);
    save_to_mapping("code", cluster.code);
    // define
    // server
    // client
    // generateCmdHandlers
    // tag
    // globalAttribute
    save_to_mapping("introducedIn", cluster.introducedIn);
    save_to_mapping("manufacturerCode", cluster.manufacturerCode);
    // singleton

    // Iterate through sdfProperties
    for (auto sdfProperty : sdfObject.sdfProperty){
        attributeType attribute;
        map_sdf_property(sdfProperty.second, attribute);
        cluster.attributes.push_back(attribute);
    }

    // Iterate through sdfActions
    for (auto sdfAction : sdfObject.sdfAction){
        commandType command;
        map_sdf_action(sdfAction.second, command);
        cluster.commands.push_back(command);
    }

    // Iterate through sdfEvents
    for (auto sdfEvent : sdfObject.sdfEvent){
        eventType event;
        map_sdf_event(sdfEvent.second, event);
        cluster.events.push_back(event);
    }
    return 0;
}

//! sdfThing -> Matter device
int map_sdf_thing(sdfThingType& sdfThing, deviceType& device)
{
    // Common qualities
    device.name = sdfThing.label;
    // domain
    // typeName
    save_to_mapping("profileId", device.profileId);
    save_to_mapping("deviceId", device.deviceId);
    // channels
    for (auto sdfObject : sdfThing.sdfObject){
        clusterType cluster;
        map_sdf_object(sdfObject.second, cluster);
        device.clusters.push_back(cluster);
    }
    //TODO: How do we handle Properties, Actions and Events of a sdfThing?
    return 0;
}

//! SDF Model + SDF Mapping -> Matter device
int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType, deviceType& device)
{
    // Make the SDF mapping global
    //MappingList.merge(sdfMappingType.map);

    std::list<clusterType> clusterList;
    if (sdfModel.sdfThing.has_value()){
        map_sdf_thing(sdfModel.sdfThing.value(), device);
    } else if (sdfModel.sdfObject.has_value()){
        // TODO: If no sdfThings are present, a new device with a single cluster has to be created
    }
    // TODO: How do we handle SDF Mappings?

    return 0;
}

//! Matter type -> SDF type
int map_matter_type(std::string& matter_type, dataQualityType& dataQuality)
{
    std::cout << "Searching for Type: " << matter_type << std::endl;
    //TODO: These seem to randomly be different from the official zcl documentation

    // Unknown type
    if (std::regex_match(matter_type, std::regex("unk"))){}

    // Data Type
    if (std::regex_match(matter_type, std::regex("data[0-9]+"))){
        if (std::regex_match(matter_type, std::regex("data8"))){
            std::cout << "Found DATA8!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data16"))){
            std::cout << "Found DATA16!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data24"))){
            std::cout << "Found DATA24!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data32"))) {
            std::cout << "Found DATA32!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data40"))) {
            std::cout << "Found DATA40!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data48"))) {
            std::cout << "Found DATA48!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data56"))) {
            std::cout << "Found DATA56!" << std::endl;
        } else if (std::regex_match(matter_type, std::regex("data64"))) {
            std::cout << "Found DATA64!" << std::endl;
        } else {
            // If the type is not a known standard type
            return -1;
        }

        return 0;
    }

    // Boolean type
    if (std::regex_search(matter_type, std::regex("bool|boolean", std::regex_constants::icase))){
        std::cout << "Found Bool!" << std::endl;
        dataQuality.type = "boolean";
        return 0;
    }

    // Bitmap type
    if (std::regex_match(matter_type, std::regex("map[0-9]+", std::regex_constants::icase))){
        std::cout << "Found Map!" << std::endl;
        return 0;
    }

    // Enum type
    if (std::regex_match(matter_type, std::regex("enum[0-9]+", std::regex_constants::icase))){
        std::cout << "Found Enum!" << std::endl;
        return 0;
    }

    // Unsigned int type
    if (std::regex_search(matter_type, std::regex("int[0-9]+u|uint[0-9]+", std::regex_constants::icase))){
        dataQuality.type = "integer";
        if (std::regex_search(matter_type, std::regex("int8u|uint8", std::regex_constants::icase))){
            dataQuality.minimum = 0;
            dataQuality.maximum = 255;
            std::cout << "Found UINT8!" << std::endl;
        }
        if (std::regex_search(matter_type, std::regex("int16u|uint16", std::regex_constants::icase))) {
            std::cout << "Found UINT16!" << std::endl;
            dataQuality.minimum = 0;
            dataQuality.maximum = 65355;
        }
        return 0;
    }

    // Signed int type
    if (std::regex_match(matter_type, std::regex("int[0-9]+", std::regex_constants::icase))){
        std::cout << "Found INT!" << std::endl;
        dataQuality.type = "integer";
        return 0;
    }

    // Array type
    if (std::regex_match(matter_type, std::regex("array", std::regex_constants::icase))){
        std::cout << "Found ARRAY!" << std::endl;
        dataQuality.type = "array";
        return 0;
    }

    // If the type is not a known standard type
    std::cout << "Found Nothing!" << std::endl;
    return -1;
};

//! Matter Access Type -> Data Quality
int map_matter_access(accessType& access, dataQualityType& dataQuality)
{
    //TODO: Can access be represented like this?
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(eventType& event, sdfEventType& sdfEvent, pugi::xml_node& event_node)
{
    sdfEvent.label = event.name;
    sdfEvent.description = event.description;
    // access
    save_to_mapping("code", event.code);
    save_to_mapping("side", event.side);
    // priority
    for (eventFieldType& eventField : event.field){
        sdfEvent.sdfOutputData.sdfChoice.insert({});
    }
    return 0;
}

//! Matter Command -> sdfAction
//! Used if a client and a server command need to be processed
int map_matter_command(commandType& client_command, commandType& server_command, sdfActionType& sdfAction, pugi::xml_node& command_node)
{
    sdfAction.label = client_command.name;
    sdfAction.description = client_command.description;

    //sdfAction.sdfData.insert()
    // access

    // Map client command arguments
    for (argType &arg: client_command.arg){
        dataQualityType dataQualities;
        // Common qualities
        dataQualities.label = arg.name;
        dataQualities.description = arg.description;

        dataQualities.default_ = arg.default_;
        dataQualities.nullable = arg.isNullable;
        map_matter_type(arg.type.name, dataQualities);
        // arraylength
        // array
        save_to_mapping("introducedIn", client_command.introducedIn);
        //save_to_mapping("removedIn", command.removedIn);
        // length
        // presentIf
        // optional
        // fieldIf
        // countArg
        sdfAction.sdfInputData.sdfChoice.insert({arg.name, dataQualities});
    }

    // Map server command arguments
    for (argType &arg: server_command.arg) {
        dataQualityType dataQualities;
        // Common qualities
        dataQualities.label = arg.name;
        dataQualities.description = arg.description;

        dataQualities.default_ = arg.default_;
        dataQualities.nullable = arg.isNullable;
        map_matter_type(arg.type.name, dataQualities);
        // arraylength
        // array
        save_to_mapping("introducedIn", client_command.introducedIn);
        //save_to_mapping("removedIn", command.removedIn);
        // length
        // presentIf
        // optional
        // fieldIf
        // countArg
        sdfAction.sdfOutputData.sdfChoice.insert({arg.name, dataQualities});
    }
    return 0;
}

//! Matter Command -> sdfAction
//! Used if only a client command needs to be processed
int map_matter_command(commandType& client_command, sdfActionType& sdfAction, pugi::xml_node& command_node)
{
    sdfAction.label = client_command.name;
    sdfAction.description = client_command.description;

    //sdfAction.sdfData.insert()
    // access

    // Map client command arguments
    for (argType &arg: client_command.arg){
        dataQualityType dataQualities;
        // Common qualities
        dataQualities.label = arg.name;
        dataQualities.description = arg.description;

        dataQualities.default_ = arg.default_;
        dataQualities.nullable = arg.isNullable;
        map_matter_type(arg.type.name, dataQualities);
        // arraylength
        // array
        save_to_mapping("introducedIn", client_command.introducedIn);
        //save_to_mapping("removedIn", command.removedIn);
        // length
        // presentIf
        // optional
        // fieldIf
        // countArg
        sdfAction.sdfInputData.sdfChoice.insert({arg.name, dataQualities});
    }
    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_action_node)
{
    auto attribute_node = sdf_action_node.append_child(attribute.name.c_str());
    attribute_node.append_attribute("code").set_value(attribute.code.c_str());
    attribute_node.append_attribute("define").set_value(attribute.define.c_str());
    if (!attribute.introducedIn.empty())
        attribute_node.append_attribute("introducedIn").set_value(attribute.introducedIn.c_str());
    if (!attribute.manufacturerCode.empty())
        attribute_node.append_attribute("manufacturerCode").set_value(attribute.manufacturerCode.c_str());

    sdfProperty.label = attribute.name;
    sdfProperty.description = attribute.description;
    if (!attribute.optional){
        sdfProperty.sdfRequired; //TODO: Create a sdfRef here
    }

    map_matter_type(attribute.type, sdfProperty);
    sdfProperty.default_ = attribute.default_;
    //sdfProperty.minLength = attribute.min; //TODO: does this match?
    //sdfProperty.maxLength = attribute.max; //TODO: does this match?
    sdfProperty.nullable = attribute.isNullable;
    // access
    save_to_mapping("code", attribute.code);
    save_to_mapping("define", attribute.define);
    save_to_mapping("introducedIn", attribute.introducedIn);
    // length
    save_to_mapping("manufacturerCode", attribute.manufacturerCode);
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    save_to_mapping("side", attribute.side);
    // array
    if (attribute.readable.has_value())
        sdfProperty.readable = attribute.readable.value();
    if (attribute.writable.has_value())
        sdfProperty.writable = attribute.writable.value();
    //sdfProperty.observable = attribute.reportable; //TODO: Does this match
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& sdf_object_node)
{
    // Append the name of the cluster to the tree
    // Also append sdfProperty, sdfAction and sdfEvent to the tree
    auto cluster_node = sdf_object_node.append_child(cluster.name.c_str());
    cluster_node.append_attribute("domain").set_value(cluster.domain.c_str());
    cluster_node.append_attribute("code").set_value(cluster.code.c_str());
    cluster_node.append_attribute("define").set_value(cluster.define.c_str());
    cluster_node.append_attribute("client").set_value(cluster.client);
    cluster_node.append_attribute("server").set_value(cluster.server);
    if (!cluster.introducedIn.empty())
        cluster_node.append_attribute("introducedIn").set_value(cluster.introducedIn.c_str());
    if (!cluster.manufacturerCode.empty())
        cluster_node.append_attribute("manufacturerCode").set_value(cluster.manufacturerCode.c_str());
    cluster_node.append_child("sdfProperty");
    cluster_node.append_child("sdfAction");
    cluster_node.append_child("sdfEvent");

    // Map individual cluster fields
    sdfObject.label = cluster.name;
    sdfObject.description = cluster.description;

    // Iterate through the attributes and map them
    auto attribute_node = cluster_node.child("sdfProperty");
    for (attributeType& attribute : cluster.attributes){
        sdfPropertyType sdfProperty;
        map_matter_attribute(attribute, sdfProperty, attribute_node);
        sdfObject.sdfProperty.insert({attribute.name, sdfProperty});
    }

    // TODO: Check if standalone server commands are possible
    // For every Matter client command there might be a corresponding server command that needs to be merged
    // Iterate through the commands and map them
    for (commandType& command : cluster.commands){
        auto command_node = cluster_node.child("sdfAction");
        bool command_mapped = false;
        if (command.source == "client"){
            for (commandType& server_command : cluster.commands){
                if (command.code == server_command.code and server_command.source == "server"){
                    sdfActionType sdfAction;
                    map_matter_command(command, server_command, sdfAction, command_node);
                    sdfObject.sdfAction.insert({command.name, sdfAction});
                    command_mapped = true;
                }
            }
            if(!command_mapped){
                sdfActionType sdfAction;
                map_matter_command(command, sdfAction, command_node);
                sdfObject.sdfAction.insert({command.name, sdfAction});
            }
        }
    }

    // Iterate through the events and map them
    auto event_node = cluster_node.child("sdfEvent");
    for (eventType& event : cluster.events){
        sdfEventType sdfEvent;
        map_matter_event(event, sdfEvent, event_node);
        sdfObject.sdfEvent.insert({event.name, sdfEvent});
    }

    return 0;
}

//! Matter Device -> SDF-Model
int map_matter_device(deviceType& device, sdfModelType& sdfModel, pugi::xml_node& sdf_thing_node)
{
    // Append a new sdfObject node to the tree
    sdf_thing_node.append_child(device.name.c_str()).append_child("sdfObject");
    auto device_node = sdf_thing_node.child(device.name.c_str());
    device_node.append_attribute("domain").set_value(device.domain.c_str());
    device_node.append_attribute("typeName").set_value(device.typeName.c_str());
    device_node.append_attribute("profileId").set_value(device.profileId.c_str());
    device_node.append_attribute("deviceId").set_value(device.deviceId.c_str());
    auto sdf_object_node = device_node.child("sdfObject");
    // channels

    // Map the information block
    sdfModel.infoBlock.title = device.name;

    // Map the namespace block
    sdfModel.namespaceBlock.namespaces.insert({"zcl", ""});
    sdfModel.namespaceBlock.defaultNamespace = "zcl";

    // Map the definition block
    sdfThingType sdfThing;
    sdfThing.label = device.name;

    // Iterate through cluster definitions for the device
    for (auto cluster : device.clusters){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject, sdf_object_node);
        sdfThing.sdfObject.insert({cluster.name, sdfObject});
    }
    sdfModel.sdfThing = sdfThing;

    return 0;
}

//! Generates a valid mapping from the reference tree
int generate_mapping(const pugi::xml_node& node, std::map<std::string, std::map<std::string, std::string>>& map)
{
    // If available, iterate through the attributes of a node
    std::map<std::string, std::string> attribute_map;
    for (auto attribute : node.attributes()) {
        attribute_map.insert({attribute.name(), attribute.value()});
    }

    // If one or more attributes are found insert them into the map
    if (!attribute_map.empty())
        // Remove the first slash as it is not needed
        // TODO: Cluster names can contain slashes (e.g. On/Off) which might render the function unusable
        // TODO: A potential workaround might be to encase such names like sdfObject/[CLUSTER_NAME]/sdfAction
        map.insert({node.path().substr(1), attribute_map});

    // Recursive call to iterate to all nodes in the tree
    for (auto child_node : node.children()) {
        generate_mapping(child_node, map);
    }

    return 0;
}

//! Matter -> SDF
int map_matter_to_sdf(deviceType& device, sdfModelType& sdfModel, sdfMappingType& sdfMapping)
{
    pugi::xml_document referenceTree;
    referenceTree.append_child("#").append_child("sdfThing");
    auto device_node = referenceTree.child("#").child("sdfThing");

    map_matter_device(device, sdfModel, device_node);

    // Initial sdfMapping mapping
    sdfMapping.infoBlock.title = device.name;
    sdfMapping.namespaceBlock.namespaces = {{"zcl", ""}};
    sdfMapping.namespaceBlock.defaultNamespace = "zcl";
    std::map<std::string, std::map<std::string, std::string>> map;
    generate_mapping(referenceTree.document_element(), map);
    sdfMapping.map = map;

    // Print the resulting tree
    simple_walker walker;
    referenceTree.traverse(walker);
    return 0;
}


