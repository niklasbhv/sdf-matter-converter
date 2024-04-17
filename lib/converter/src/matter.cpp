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

#include <list>
#include <iostream>
#include <pugixml.hpp>
#include "matter.h"

int parseConformance(const pugi::xml_node& conformance_node, conformanceType& conformance)
{
    // TODO: These can be combined with logic operations to create complex expressions, should be caught
    // Mandatory conform
    if (!conformance_node.child("mandatoryConform").empty()) {
        conformance.mandatory = true;
    }
    // Optional conform
    else if (!conformance_node.child("optionalConform").empty()) {
        conformance.optional = true;
    }
    // TODO: Check the attribute names for these
    // Provisional conform
    else if (!conformance_node.child("provisionalConform").empty()) {
        conformance.provisional = true;
    }
    // Deprecated conform
    else if (!conformance_node.child("deprecatedConform").empty()) {
        conformance.deprecated = true;
    }
    // Disallowed conform
    else if (!conformance_node.child("disallowedConform").empty()) {
        conformance.disallowed = true;
    }

    return 0;
}

int parseEnumItems(const pugi::xml_node& enum_node, std::list<enumItemType>& items)
{
    // Iterate through all enum items and parse them individually
    for (const auto& enum_item_node : enum_node.children()) {
        enumItemType item;
        item.value = enum_item_node.attribute("value").as_int();
        item.name = enum_item_node.attribute("name").value();
        item.summary = enum_item_node.attribute("summary").value();
        parseConformance(enum_item_node, item.conformance);
        items.push_back(item);
    }

    return 0;
}

int parseBitfields(const pugi::xml_node& bitmap_node, std::list<bitmapBitfieldType>& bitfields)
{
    // Iterate through all bitfields and parse them individually
    for (const auto& bitfield_node : bitmap_node.children()) {
        bitmapBitfieldType bitfield;
        bitfield.bit = bitfield_node.attribute("bit").as_int();
        bitfield.name = bitfield_node.attribute("name").value();
        bitfield.summary = bitfield_node.attribute("summary").value();
        parseConformance(bitfield_node, bitfield.conformance);
        bitfields.push_back(bitfield);
    }

    return 0;
}

int parseStructFields(const pugi::xml_node& struct_node, std::list<structFieldType>& struct_fields)
{
    // Iterate through all struct fields and parse them individually
    for (const auto& struct_field_node : struct_node.children()) {
        structFieldType struct_field;
        struct_field.id = struct_field_node.attribute("id").as_int();
        struct_field.name = struct_field_node.attribute("name").value();
        struct_field.type = struct_field_node.attribute("type").value();
        parseConformance(struct_field_node, struct_field.conformance);
        struct_fields.push_back(struct_field);
    }

    return 0;
}

int parseFeatureMap(const pugi::xml_node& feature_map_node, std::list<featureMapType>& featureMapList)
{
    // Iterate through all features and parse them individually
    for (const auto& feature : feature_map_node.children()) {
        featureMapType featureMap;
        featureMap.bit = feature.attribute("bit").as_int();
        parseConformance(feature_map_node, featureMap.conformance);
        featureMap.code = feature.attribute("code").value();
        featureMap.name = feature.attribute("name").value();
        featureMap.summary = feature.attribute("summary").value();
        featureMapList.push_back(featureMap);
    }

    return 0;
}

int parseAccess(const pugi::xml_node& access_node, accessType& access)
{
    if (!access_node.attribute("read").empty())
        access.read = access_node.attribute("read").as_bool();
    if (!access_node.attribute("write").empty())
        access.write = access_node.attribute("write").as_bool();
    // TODO: Seems like read and write have separate privileges, check specification

    return 0;
}

int parseEvent(const pugi::xml_node& event_node, eventType& event)
{
    event.id = event_node.attribute("id").as_int();
    event.name = event_node.attribute("name").value();
    parseConformance(event_node, event.conformance);
    parseAccess(event_node, event.access);
    // TODO: Where is summary defined?
    // summary
    // TODO: Priority needs to be cast from string
    // priority
    // TODO: This definitely needs further additions

    return 0;
}

int parseCommand(const pugi::xml_node& command_node, commandType& command)
{
    command.id = command_node.attribute("id").as_int();
    command.name = command_node.attribute("name").value();
    parseConformance(command_node, command.conformance);
    parseAccess(command_node.child("access"), command.access);
    // TODO: Where is summary defined?
    // summary
    // default
    command.direction = command_node.attribute("direction").value();
    command.response = command_node.attribute("response").value();

    return 0;
}

int parseAttribute(const pugi::xml_node& attribute_node, attributeType& attribute)
{
    attribute.id = attribute_node.attribute("id").as_int();
    attribute.name = attribute_node.attribute("name").value();
    std::cout << attribute.name << std::endl;
    parseConformance(attribute_node, attribute.conformance);
    parseAccess(attribute_node.child("access"), attribute.access);
    // TODO: Where is summary defined?
    // summary
    attribute.type = attribute_node.attribute("type").value();
    // TODO: As these are optional, check their presence to ensure the correct function of optional
    auto quality_node = attribute_node.child("quality");
    attribute.qualities.nullable = quality_node.attribute("nullable").as_bool();
    attribute.qualities.non_volatile = quality_node.attribute("nonVolatile").as_bool();
    attribute.qualities.fixed = quality_node.attribute("fixed").as_bool();
    attribute.qualities.scene = quality_node.attribute("scene").as_bool();
    attribute.qualities.reportable = quality_node.attribute("reportable").as_bool();
    attribute.qualities.changes_omitted = quality_node.attribute("changeOmitted").as_bool();
    attribute.qualities.singleton = quality_node.attribute("singleton").as_bool();
    attribute.default_ = quality_node.attribute("default").value();

    return 0;
}

int parseCluster(const pugi::xml_node& cluster_xml, clusterType& cluster)
{
    // Search for the matching cluster inside the cluster xml definitions
    //for (const auto& cluster_node : cluster_xml.children("cluster")) {
    // Match the cluster definitions with their unique id
    if (cluster_xml.attribute("id").as_int() == cluster.id) {
        std::cout << "MATCHED CLUSTER" << std::endl;
        cluster.revision = cluster_xml.attribute("revision").as_int();

        // Iterate through all revisions and parse them individually
        for (const auto& revision_node : cluster_xml.child("revisionHistory").children()) {
            cluster.revision_history.insert({revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
        }
        // classification
        // Iterate through all attributes and parse them individually
        for (const auto& attribute_node : cluster_xml.child("attributes").children()) {
            attributeType attribute;
            parseAttribute(attribute_node, attribute);
            cluster.attributes.push_back(attribute);
        }

        // Iterate through all commands and parse them individually
        for (const auto& command_node : cluster_xml.child("commands").children()) {
            commandType command;
            parseCommand(command_node, command);
            cluster.commands.push_back(command);
        }

        // Iterate through all events and parse them individually
        for (const auto& event_node : cluster_xml.child("events").children()) {
            eventType event;
            parseEvent(event_node, event);
            cluster.events.push_back(event);
        }
    }
    //}

    return 0;
}

int parseDevice(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device)
{
    device.id = device_xml.attribute("id").as_int();
    device.name = device_xml.attribute("name").value();
    device.revision = device_xml.attribute("revision").as_int();
    std::cout << device.name << std::endl;

    // Iterate through all revisions and parse them individually
    for (const auto& revision_node : device_xml.child("revisionHistory").children()) {
        device.revision_history.insert({revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
    }

    // TODO:  Insert classification
    // Parse the feature map
    parseFeatureMap(device_xml, device.features);

    // Parse all data types and add them to a map
    auto data_type_node = device_xml.child("dataTypes");

    // Iterate through all enums and parse them individually
    for (const auto& enum_node : data_type_node.children("enum")) {
        std::list<enumItemType> items;
        parseEnumItems(enum_node, items);
        device.enums[enum_node.attribute("name").value()] = items;
    }

    // Iterate through all bitmaps and parse them individually
    for (const auto& bitmap_node : data_type_node.children("bitmap")) {
        std::list<bitmapBitfieldType> bitfields;
        parseBitfields(bitmap_node, bitfields);
        device.bitmaps[bitmap_node.attribute("name").value()] = bitfields;
    }

    // Iterate through all structs and parse them individually
    for (const auto& struct_node : data_type_node.children("struct")) {
        std::list<structFieldType> struct_fields;
        parseStructFields(struct_node, struct_fields);
        device.structs[struct_node.attribute("name").value()] = struct_fields;
    }

    // Iterate through all clusters needed by the device and parse them individually
    for (const auto& cluster_node : device_xml.child("clusters").children("cluster")) {
        clusterType cluster;
        cluster.id = cluster_node.attribute("id").as_int();
        cluster.name = cluster_node.attribute("name").value();
        // TODO: Add conform here
        // TODO: Add side
        // TODO: Check for the return value
        parseCluster(cluster_xml, cluster);
        device.clusters.push_back(cluster);
    }

    return 0;
}

int serializeEvent(const eventType& event, pugi::xml_node& event_xml)
{
    return 0;
}

int serializeCommand(const commandType& command, pugi::xml_node& command_xml)
{
    return 0;
}

int serializeAttribute(const attributeType& attribute, pugi::xml_node& attribute_xml)
{
    return 0;
}

int serializeCluster(const clusterType& cluster, pugi::xml_node& cluster_xml)
{
    return 0;
}

int serializeDevice(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml)
{
    return 0;
}
