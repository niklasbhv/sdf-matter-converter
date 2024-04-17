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

std::map<std::string, std::map<std::string, std::string>> reference_map;

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

template <typename T>

// TODO: Check how this function should behave on an not available mapping
// Generically typed for now as the mapping will later contain different types
int resolve_mapping(const std::string& reference, const std::string& entry, T& result)
{
    // Be aware that this function removes the first character from the reference string
    result = reference_map.at(reference.substr(1)).at(entry);
    return 0;
}

//! sdfEvent -> Matter event
int map_sdf_event(const sdfEventType& sdfEvent, eventType& event)
{
    return 0;
}

//! sdfAction -> Matter command
int map_sdf_action(const sdfActionType& sdfAction, commandType& command)
{
    return 0;
}

//! sdfProperty -> Matter attribute
int map_sdf_property(const sdfPropertyType& sdfProperty, attributeType& attribute, pugi::xml_node& sdf_property_node)
{
    return 0;
}

//! sdfObject -> Matter cluster
int map_sdf_object(const sdfObjectType& sdfObject, clusterType& cluster, pugi::xml_node& sdf_object_node)
{
    return 0;
}

//! sdfThing -> Matter device
int map_sdf_thing(const sdfThingType& sdfThing, deviceType& device, pugi::xml_node& sdf_thing_node)
{
    // Add the current sdfThing to the reference tree
    auto current_thing_node = sdf_thing_node.append_child(sdfThing.label.c_str());
    // TODO: Find a way to make the mapping compatible with multiple types
    // resolve_mapping(current_thing_node.path(), "id" ,device.id);
    device.name = sdfThing.label;
    // conformance
    // access
    device.summary = sdfThing.description;
    // revision
    // revision_history

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdfObject : sdfThing.sdfObject) {
        clusterType cluster;
        auto sdf_object_node = current_thing_node.append_child("sdfObject");
        map_sdf_object(sdfObject.second, cluster, sdf_object_node);
        device.clusters.push_back(cluster);
    }

    return 0;
}

//! SDF Model + SDF Mapping -> Matter device
int map_sdf_to_matter(const sdfModelType& sdfModel, const sdfMappingType& sdfMappingType, deviceType& device)
{
    // Make the mapping a global variable
    if (!sdfMappingType.map.empty())
        reference_map = sdfMappingType.map;

    // Initialize a reference tree used to resolve json references
    pugi::xml_document reference_tree;
    auto reference_root_node = reference_tree.append_child("#");

    if (sdfModel.sdfThing.has_value()){
        auto sdf_thing_node = reference_tree.append_child("sdfThing");
        map_sdf_thing(sdfModel.sdfThing.value(), device, sdf_thing_node);
    } else if (sdfModel.sdfObject.has_value()){
        auto sdf_object_node = reference_tree.append_child("sdfObject");
        // TODO: Special case, initialize a device with a single cluster
    }

    return 0;
}

//! Matter type -> SDF type
int map_matter_type(const std::string& matter_type, dataQualityType& dataQuality)
{
    // TODO: Custom types should be handled here
    // TODO: Maybe also set the default value here?
    if (matter_type == "bool") {
        dataQuality.type = "boolean";
    }
    if (matter_type == "single") {
        dataQuality.type = "number";
    }
    if (matter_type == "double") {
        dataQuality.type = "number";
    }
    if (matter_type == "octstr") {
        dataQuality.type = "string";
    }
    if (matter_type == "list") {
        dataQuality.type = "array";
    }
    if (matter_type == "struct") {
        dataQuality.type = "object";
    }
    if (matter_type.substr(0, 3) == "map") {
        dataQuality.type = "string";
    }
    if (matter_type.substr(0, 3) == "int") {
        // TODO: These boundaries change if the corresponding value is nullable
        dataQuality.type = "integer";
        dataQuality.minimum = 0;
        if (matter_type.substr(3) == "8") {
            dataQuality.maximum = 2^8-1;
        } else if (matter_type.substr(3) == "16") {
            dataQuality.maximum = 2^16-1;
        } else if (matter_type.substr(3) == "24") {
            dataQuality.maximum = 2^24-1;
        } else if (matter_type.substr(3) == "32") {
            dataQuality.maximum = 2^32-1;
        } else if (matter_type.substr(3) == "40") {
            dataQuality.maximum = 2^40-1;
        } else if (matter_type.substr(3) == "48") {
            dataQuality.maximum = 2^48-1;
        } else if (matter_type.substr(3) == "56") {
            dataQuality.maximum = 2^56-1;
        } else if (matter_type.substr(3) == "64") {
            dataQuality.maximum = 2^64-1;
        }
    }
    if (matter_type.substr(0, 4) == "uint") {
        dataQuality.type = "integer";
        if (matter_type.substr(4) == "8") {
            dataQuality.minimum = -2^7;
            dataQuality.maximum = 2^7-1;
        } else if (matter_type.substr(4) == "16") {
            dataQuality.minimum = -2^15;
            dataQuality.maximum = 2^15-1;
        } else if (matter_type.substr(4) == "24") {
            dataQuality.minimum = -2^23;
            dataQuality.maximum = 2^23-1;
        } else if (matter_type.substr(4) == "32") {
            dataQuality.minimum = -2^31;
            dataQuality.maximum = 2^31-1;
        } else if (matter_type.substr(4) == "40") {
            dataQuality.minimum = -2^39;
            dataQuality.maximum = 2^39-1;
        } else if (matter_type.substr(4) == "48") {
            dataQuality.minimum = -2^47;
            dataQuality.maximum = 2^47-1;
        } else if (matter_type.substr(4) == "56") {
            dataQuality.minimum = -2^55;
            dataQuality.maximum = 2^55-1;
        } else if (matter_type.substr(4) == "64") {
            dataQuality.minimum = -2^63;
            dataQuality.maximum = 2^63-1;
        }
    }

    // If the type is not a known standard type
    std::cout << "Found: " << matter_type << std::endl;
    return 0;
};

//! Matter Access Type -> Data Quality
int map_matter_access(const accessType& access, dataQualityType& dataQuality)
{
    //TODO: Can access be represented like this?
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(const eventType& event, sdfEventType& sdfEvent, pugi::xml_node& sdf_event_node)
{
    // Append the event node to the tree
    auto event_node = sdf_event_node.append_child(event.name.c_str());

    // event.id
    sdfEvent.label = event.name;
    // event.conformance
    // event.access
    sdfEvent.description = event.summary;
    // event.priority
    // event.number
    // event.timestamp
    // event.data

    return 0;
}

//! Matter Command -> sdfAction
//! Used if a client and a server command need to be processed
int map_matter_command(const commandType& client_command, commandType& server_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    return 0;
}

//! Matter Command -> sdfAction
//! Used if only a client command needs to be processed
int map_matter_command(const commandType& client_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    // client_command.id
    sdfAction.label = client_command.name;
    // client_command.conformance
    // client_command.access
    sdfAction.description = client_command.summary;
    // client_command.default_
    // client_command.direction
    // client_command.response

    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(const attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node)
{
    // Append the attribute node to the tree
    auto attribute_node = sdf_property_node.append_child(attribute.name.c_str());

    // attribute.id
    sdfProperty.label = attribute.name;
    // attribute.conformance
    // attribute.access
    sdfProperty.description = attribute.summary;

    // Map the Matter type to a fitting SDF type
    map_matter_type(attribute.type, sdfProperty);
    if (attribute.qualities.nullable.has_value())
        // TODO: In this case the boundaries for some types have to be changed
        sdfProperty.nullable = attribute.qualities.nullable.value();
    // attribute.qualities.non_volatile
    // attribute.qualities.fixed
    // attribute.qualities.scene
    // attribute.qualities.reportable
    // attribute.qualities.changed_omitted
    // attribute.qualities.singleton
    sdfProperty.default_ = attribute.default_;

    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(const clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& sdf_object_node)
{
    // Append the name of the cluster to the tree
    // Also append sdfProperty, sdfAction and sdfEvent to the tree
    auto cluster_node = sdf_object_node.append_child(cluster.name.c_str());
    cluster_node.append_child("sdfProperty");
    cluster_node.append_child("sdfAction");
    cluster_node.append_child("sdfEvent");

    // cluster.id
    sdfObject.label = cluster.name;
    // cluster.conformance
    // cluster.access
    sdfObject.description = cluster.summary;
    // cluster.revision -> sdfData
    // cluster.revision_history -> sdfData

    // Iterate through the attributes and map them
    auto attribute_node = cluster_node.child("sdfProperty");
    for (const auto& attribute : cluster.attributes){
        sdfPropertyType sdfProperty;
        map_matter_attribute(attribute, sdfProperty, attribute_node);
        sdfObject.sdfProperty.insert({attribute.name, sdfProperty});
    }

    // Iterate through the commands and map them
    auto command_node = cluster_node.child("sdfAction");
    for (const auto& command : cluster.commands){
        sdfActionType sdfAction;
        map_matter_command(command, sdfAction, command_node);
        sdfObject.sdfAction.insert({command.name, sdfAction});
    }

    // Iterate through the events and map them
    auto event_node = cluster_node.child("sdfEvent");
    for (const auto& event : cluster.events){
        sdfEventType sdfEvent;
        map_matter_event(event, sdfEvent, event_node);
        sdfObject.sdfEvent.insert({event.name, sdfEvent});
    }

    return 0;
}

//! Matter Device -> SDF-Model (sdfThing)
int map_matter_device(const deviceType& device, sdfModelType& sdfModel, pugi::xml_node& sdf_thing_node)
{
    // Append a new sdfObject node to the tree
    sdf_thing_node.append_child(device.name.c_str()).append_child("sdfObject");
    auto device_node = sdf_thing_node.child(device.name.c_str());
    device_node.append_attribute("id").set_value(device.id);
    // device.classification -> sdfMapping
    // device.features -> sdfData
    // device.enums -> sdfData
    // device.bitmaps -> sdfData
    // device.structs -> sdfData
    // device.conformance
    // device.access
    // TODO: We need to be able to create a JSON object for more complex structures like revisionHistory
    // device.revisionHistory -> sdfData

    // Map the information block
    sdfModel.infoBlock.title = device.name;
    sdfModel.infoBlock.description = device.summary;
    sdfModel.infoBlock.version = std::to_string(device.revision);
    // sdfModel.infoBlock.modified
    // sdfModel.infoBlock.copyright <-- Maybe parse from the comment?
    // sdfModel.infoBlock.license <-- Maybe parse from the comment?
    // sdfModel.infoBlock.features <-- This can definitely be parsed from the features section
    // sdfModel.infoBlock.$comment

    // Map the namespace block
    sdfModel.namespaceBlock.namespaces.insert({"zcl", "https://zcl.example.com/sdf"});
    sdfModel.namespaceBlock.defaultNamespace = "zcl";

    // Map the definition block
    sdfThingType sdfThing;
    sdfThing.label = device.name;
    sdfThing.description = device.summary;
    // sdfThing.$comment
    // sdfThing.sdfRef
    // sdfThing.sdfRequired
    // sdfThing.sdfProperty
    // sdfThing.sdfAction
    // sdfThing.sdfEvent
    // sdfThing.minItems
    // sdfThing.maxItems
    // Iterate through cluster definitions for the device
    auto sdf_object_node = device_node.child("sdfObject");
    for (const auto& cluster : device.clusters){
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
int map_matter_to_sdf(const deviceType& device, sdfModelType& sdfModel, sdfMappingType& sdfMapping)
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


