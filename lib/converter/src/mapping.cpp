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
    result = reference_map.at(reference).at(entry);
    return 0;
}

//! sdfEvent -> Matter event
int map_sdf_event(sdfEventType& sdfEvent, eventType& event)
{
    return 0;
}

//! sdfAction -> Matter command
int map_sdf_action(sdfActionType& sdfAction, commandType& command)
{
    return 0;
}

//! sdfProperty -> Matter attribute
int map_sdf_property(sdfPropertyType& sdfProperty, attributeType& attribute, pugi::xml_node& sdf_property_node)
{
    return 0;
}

//! sdfObject -> Matter cluster
int map_sdf_object(sdfObjectType& sdfObject, clusterType& cluster, pugi::xml_node& sdf_object_node)
{
    return 0;
}

//! sdfThing -> Matter device
int map_sdf_thing(sdfThingType& sdfThing, deviceType& device, pugi::xml_node& sdf_thing_node)
{
    return 0;
}

//! SDF Model + SDF Mapping -> Matter device
int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType, deviceType& device)
{
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
int map_matter_event(eventType& event, sdfEventType& sdfEvent, pugi::xml_node& sdf_event_node)
{
    return 0;
}

//! Matter Command -> sdfAction
//! Used if a client and a server command need to be processed
int map_matter_command(commandType& client_command, commandType& server_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    return 0;
}

//! Matter Command -> sdfAction
//! Used if only a client command needs to be processed
int map_matter_command(commandType& client_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node)
{
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& sdf_object_node)
{
    return 0;
}

//! Matter Device -> SDF-Model
int map_matter_device(deviceType& device, sdfModelType& sdfModel, pugi::xml_node& sdf_thing_node)
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


