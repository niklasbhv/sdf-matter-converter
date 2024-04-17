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
int map_matter_type(std::string& matter_type, dataQualityType& dataQuality)
{
    // TODO: Custom types should be handled here
    // TODO: Maybe also set the default value here?
    if (matter_type == "bool") {

    }
    if (matter_type == "single") {

    }
    if (matter_type == "double") {

    }
    if (matter_type == "octstr") {

    }
    if (matter_type == "list") {

    }
    if (matter_type == "struct") {

    }
    if (matter_type.substr(0, 3) == "map") {

    }
    if (matter_type.substr(0, 3) == "int") {

    }
    if (matter_type.substr(0, 4) == "uint") {

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
    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(const attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node)
{
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(const clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& sdf_object_node)
{
    return 0;
}

//! Matter Device -> SDF-Model
int map_matter_device(const deviceType& device, sdfModelType& sdfModel, pugi::xml_node& sdf_thing_node)
{
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


