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
#include <cstring>
#include <pugixml.hpp>
#include "matter.h"

int parse_other_qualities(const pugi::xml_node& quality_node, otherQualityType& otherQualities){
    if (!quality_node.attribute("nullable").empty())
        otherQualities.nullable = quality_node.attribute("nullable").as_bool();
    if (!quality_node.attribute("persistence").empty()) {
        // TODO: It seems like the qualities fixed and non-volatile are combined with persistence
        if (strcmp(quality_node.attribute("persistence").value(), "fixed") == 0) {
            otherQualities.fixed = true;
        } else if (strcmp(quality_node.attribute("persistence").value(), "volatile") == 0) {
            otherQualities.non_volatile = false;
        } else if (strcmp(quality_node.attribute("persistence").value(), "nonVolatile") == 0){
            otherQualities.non_volatile = true;
        }
    }
    if (!quality_node.attribute("fixed").empty()) //Check this
        otherQualities.fixed = quality_node.attribute("fixed").as_bool();
    if (!quality_node.attribute("scene").empty())
        otherQualities.scene = quality_node.attribute("scene").as_bool();
    if (!quality_node.attribute("reportable").empty())
        otherQualities.reportable = quality_node.attribute("reportable").as_bool();
    if (!quality_node.attribute("changesOmitted").empty())
        otherQualities.changes_omitted = quality_node.attribute("changesOmitted").as_bool();
    if (!quality_node.attribute("singleton").empty())
        otherQualities.singleton = quality_node.attribute("singleton").as_bool();
    if (!quality_node.attribute("diagnostics").empty()) //Check this
        otherQualities.diagnostics = quality_node.attribute("diagnostics").as_bool();
    if (!quality_node.attribute("largeMessage").empty()) //Check this
        otherQualities.large_message = quality_node.attribute("largeMessage").as_bool();
    if (!quality_node.attribute("quieterReporting").empty()) //Check this
        otherQualities.quieter_reporting = quality_node.attribute("quieterReporting").as_bool();
    return 0;
}

int parse_conformance(const pugi::xml_node& conformance_node, conformanceType& conformance)
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

int parse_event_records(const pugi::xml_node& event_record_node, eventRecordType& eventRecord)
{
    return 0;
}

int parse_enum_items(const pugi::xml_node& enum_node, std::list<enumItemType>& items)
{
    // Iterate through all enum items and parse them individually
    for (const auto& enum_item_node : enum_node.children()) {
        enumItemType item;
        item.value = enum_item_node.attribute("value").as_int();
        item.name = enum_item_node.attribute("name").value();
        item.summary = enum_item_node.attribute("summary").value();
        parse_conformance(enum_item_node, item.conformance);
        items.push_back(item);
    }

    return 0;
}

int parse_bitfields(const pugi::xml_node& bitmap_node, std::list<bitmapBitfieldType>& bitfields)
{
    // Iterate through all bitfields and parse them individually
    for (const auto& bitfield_node : bitmap_node.children()) {
        bitmapBitfieldType bitfield;
        bitfield.bit = bitfield_node.attribute("bit").as_int();
        bitfield.name = bitfield_node.attribute("name").value();
        bitfield.summary = bitfield_node.attribute("summary").value();
        parse_conformance(bitfield_node, bitfield.conformance);
        bitfields.push_back(bitfield);
    }

    return 0;
}

int parse_struct_fields(const pugi::xml_node& struct_node, std::list<structFieldType>& struct_fields)
{
    // Iterate through all struct fields and parse them individually
    for (const auto& struct_field_node : struct_node.children()) {
        structFieldType struct_field;
        struct_field.id = struct_field_node.attribute("id").as_int();
        struct_field.name = struct_field_node.attribute("name").value();
        parse_conformance(struct_field_node, struct_field.conformance);
        struct_fields.push_back(struct_field);
    }

    return 0;
}

int parse_feature_map(const pugi::xml_node& feature_map_node, std::list<featureMapType>& featureMapList)
{
    // Iterate through all features and parse them individually
    for (const auto& feature : feature_map_node.children()) {
        featureMapType featureMap;
        featureMap.bit = feature.attribute("bit").as_int();
        parse_conformance(feature_map_node, featureMap.conformance);
        featureMap.code = feature.attribute("code").value();
        featureMap.name = feature.attribute("name").value();
        featureMap.summary = feature.attribute("summary").value();
        featureMapList.push_back(featureMap);
    }

    return 0;
}

int parse_access(const pugi::xml_node& access_node, accessType& access)
{
    if (!access_node.attribute("read").empty())
        access.read = access_node.attribute("read").as_bool();
    if (!access_node.attribute("write").empty())
        access.write = access_node.attribute("write").as_bool();
    // TODO: Seems like read and write have separate privileges, check specification

    return 0;
}

int parse_event(const pugi::xml_node& event_node, eventType& event)
{
    event.id = event_node.attribute("id").as_int();
    event.name = event_node.attribute("name").value();
    parse_conformance(event_node, event.conformance);
    parse_access(event_node, event.access);
    event.summary = event_node.attribute("summary").value();
    event.priority = event_node.attribute("priority").value();
    parse_other_qualities(event_node.child("quality"), event.quality);
    for (auto record_node : event_node.children("field")) {
        eventRecordType eventRecord;
        parse_event_records(record_node, eventRecord);
        event.event_records.push_back(eventRecord);
    }
    return 0;
}

int parse_command(const pugi::xml_node& command_node, commandType& command)
{
    command.id = command_node.attribute("id").as_int();
    command.name = command_node.attribute("name").value();
    parse_conformance(command_node, command.conformance);
    parse_access(command_node.child("access"), command.access);
    // TODO: Where is summary defined?
    // summary
    // default
    command.direction = command_node.attribute("direction").value();
    command.response = command_node.attribute("response").value();

    return 0;
}

int parse_attribute(const pugi::xml_node& attribute_node, attributeType& attribute)
{
    attribute.id = attribute_node.attribute("id").as_int();
    attribute.name = attribute_node.attribute("name").value();
    std::cout << attribute.name << std::endl;
    parse_conformance(attribute_node, attribute.conformance);
    parse_access(attribute_node.child("access"), attribute.access);
    // TODO: Where is summary defined?
    // summary
    attribute.type = attribute_node.attribute("type").value();
    // TODO: As these are optional, check their presence to ensure the correct function of optional
    auto quality_node = attribute_node.child("quality");
    attribute.quality.nullable = quality_node.attribute("nullable").as_bool();
    attribute.quality.non_volatile = quality_node.attribute("nonVolatile").as_bool();
    attribute.quality.fixed = quality_node.attribute("fixed").as_bool();
    attribute.quality.scene = quality_node.attribute("scene").as_bool();
    attribute.quality.reportable = quality_node.attribute("reportable").as_bool();
    attribute.quality.changes_omitted = quality_node.attribute("changeOmitted").as_bool();
    attribute.quality.singleton = quality_node.attribute("singleton").as_bool();
    attribute.default_ = quality_node.attribute("default").value();

    return 0;
}

int parse_cluster(const pugi::xml_node& cluster_xml, clusterType& cluster)
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
            parse_attribute(attribute_node, attribute);
            cluster.attributes.push_back(attribute);
        }

        // Iterate through all commands and parse them individually
        for (const auto& command_node : cluster_xml.child("commands").children()) {
            commandType command;
            parse_command(command_node, command);
            cluster.commands.push_back(command);
        }

        // Iterate through all events and parse them individually
        for (const auto& event_node : cluster_xml.child("events").children()) {
            eventType event;
            parse_event(event_node, event);
            cluster.events.push_back(event);
        }
    }
    //}

    return 0;
}

int parse_device(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device)
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
    parse_feature_map(device_xml, device.features);

    // Parse all data types and add them to a map
    auto data_type_node = device_xml.child("dataTypes");

    // Iterate through all enums and parse them individually
    for (const auto& enum_node : data_type_node.children("enum")) {
        std::list<enumItemType> items;
        parse_enum_items(enum_node, items);
        device.enums[enum_node.attribute("name").value()] = items;
    }

    // Iterate through all bitmaps and parse them individually
    for (const auto& bitmap_node : data_type_node.children("bitmap")) {
        std::list<bitmapBitfieldType> bitfields;
        parse_bitfields(bitmap_node, bitfields);
        device.bitmaps[bitmap_node.attribute("name").value()] = bitfields;
    }

    // Iterate through all structs and parse them individually
    for (const auto& struct_node : data_type_node.children("struct")) {
        std::list<structFieldType> struct_fields;
        parse_struct_fields(struct_node, struct_fields);
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
        parse_cluster(cluster_xml, cluster);
        device.clusters.push_back(cluster);
    }

    return 0;
}

int serialize_conformance(const conformanceType& conformance, pugi::xml_node& current_node)
{
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value())
            current_node.append_child("mandatoryConformance");
    } else if (conformance.optional.has_value()) {
        if (conformance.optional.value())
            current_node.append_child("optionalConformance");
    } else if (conformance.provisional.has_value()) {
        if (conformance.provisional.value())
            current_node.append_child("provisionalConformance");
    } else if (conformance.deprecated.has_value()) {
        if (conformance.deprecated.value())
            current_node.append_child("deprecatedConformance");
    } else if (conformance.disallowed.has_value()) {
        if (conformance.disallowed.value())
            current_node.append_child("disallowedConformance");
    }
    // expression
    return 0;
}

int serialize_access(const accessType& access, pugi::xml_node& current_node)
{
    // TODO: What happens if none of the attributes are set?
    auto access_node = current_node.append_child("access");

    if (access.read.has_value())
        access_node.append_attribute("read").set_value(access.read.value());
    if (access.write.has_value())
        access_node.append_attribute("write").set_value(access.write.value());
    // TODO: The format of the remaining types has to be checked

    return 0;
}

int serialize_event(const eventType& event, pugi::xml_node& event_xml)
{
    return 0;
}

int serialize_command(const commandType& command, pugi::xml_node& command_xml)
{
    auto command_node = command_xml.append_child("command");

    command_node.append_attribute("id").set_value(command.id);
    command_node.append_attribute("name").set_value(command.name.c_str());
    serialize_conformance(command.conformance, command_node);
    serialize_access(command.access, command_node);
    if (!command.summary.empty())
        command_node.append_attribute("summary").set_value(command.summary.c_str());
    if (!command.default_.empty())
        command_node.append_attribute("default").set_value(command.default_.c_str());
    command_node.append_attribute("direction").set_value(command.direction.c_str());
    command_node.append_attribute("response").set_value(command.response.c_str());

    return 0;
}

int serialize_attribute(const attributeType& attribute, pugi::xml_node& attribute_xml)
{
    auto attribute_node = attribute_xml.append_child("attribute");

    attribute_node.append_attribute("id").set_value(attribute.id);
    attribute_node.append_attribute("name").set_value(attribute.name.c_str());
    serialize_conformance(attribute.conformance, attribute_node);
    serialize_access(attribute.access, attribute_node);
    if (!attribute.summary.empty())
        attribute_node.append_attribute("summary").set_value(attribute.summary.c_str());
    attribute_node.attribute("type").set_value(attribute.type.c_str());
    // qualities
    attribute_node.attribute("default").set_value(attribute.default_.c_str());

    return 0;
}

int serialize_cluster(const clusterType& cluster, pugi::xml_node& cluster_xml)
{
    // Create the cluster node
    auto cluster_node = cluster_xml.append_child("cluster");

    cluster_node.append_attribute("id").set_value(cluster.id);
    cluster_node.append_attribute("name").set_value(cluster.name.c_str());
    serialize_conformance(cluster.conformance, cluster_node);
    serialize_access(cluster.access, cluster_node);
    if (!cluster.summary.empty())
        cluster_node.append_attribute("summary").set_value(cluster.summary.c_str());
    // classification

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = cluster_node.append_child("revisionHistory");
    for (const auto& revision : cluster.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    // Iterate through all attributes and serialize them individually
    auto attribute_node = cluster_node.append_child("attributes");
    for (const auto& attribute : cluster.attributes) {
        serialize_attribute(attribute, attribute_node);
    }

    // Iterate through all commands and serialize them individually
    auto command_node = cluster_node.append_child("commands");
    for (const auto& command : cluster.commands) {
        serialize_command(command, command_node);
    }

    // Iterate through all events and serialize them individually
    auto event_node = cluster_node.append_child("events");
    for (const auto& event : cluster.events) {
        serialize_event(event, event_node);
    }

    return 0;
}

int serialize_device(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml)
{
    auto device_node = device_xml.append_child("deviceType");

    device_node.append_attribute("id").set_value(device.id);
    device_node.append_attribute("name").set_value(device.name.c_str());
    serialize_conformance(device.conformance, device_node);
    serialize_access(device.access, device_node);
    if (!device.summary.empty())
        device_node.append_attribute("summary").set_value(device.summary.c_str());
    device_node.append_attribute("revision").set_value(device.revision);

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = device_node.append_child("revisionHistory");
    for (const auto& revision : device.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }
    // classification
    // Iterate through all clusters and serialize them individually
    auto clusters_node = device_node.append_child("clusters");
    for (const auto& cluster : device.clusters) {
        serialize_cluster(cluster, cluster_xml);
    }

    // Iterate through all features and parse them individually
    auto features_node = device_node.append_child("features");
    for (const auto& feature : device.features) {
        // feature
    }
    // enums
    // bitmaps
    // structs

    return 0;
}
