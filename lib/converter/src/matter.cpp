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

/**
 *
 *
 * @param quality_node
 * @param otherQualities
 * @return
 */
int parse_other_qualities(const pugi::xml_node& quality_node, otherQualityType& otherQualities)
{
    if (!quality_node.attribute("nullable").empty())
        otherQualities.nullable = quality_node.attribute("nullable").as_bool();

    if (!quality_node.attribute("persistence").empty()) {
        if (strcmp(quality_node.attribute("persistence").value(), "fixed") == 0) {
            otherQualities.fixed = true;
        } else if (strcmp(quality_node.attribute("persistence").value(), "volatile") == 0) {
            otherQualities.non_volatile = false;
        } else if (strcmp(quality_node.attribute("persistence").value(), "nonVolatile") == 0){
            otherQualities.non_volatile = true;
        }
    }
    if (!quality_node.attribute("scene").empty())
        otherQualities.scene = quality_node.attribute("scene").as_bool();

    if (!quality_node.attribute("reportable").empty())
        otherQualities.reportable = quality_node.attribute("reportable").as_bool();

    if (!quality_node.attribute("changeOmitted").empty())
        otherQualities.change_omitted = quality_node.attribute("changeOmitted").as_bool();

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

int parse_constraints(const pugi::xml_node& constraint_node, constraintType& constraint)
{
    constraint.type = constraint_node.attribute("type").value();

    // Constraint is defined in the description section
    if (constraint.type == "desc") {
        // In this case, there is nothing more to parse
    }

    // Composed device type constraints
    // allowed
    // between
    // min
    // max

    // Numeric constraints
    else if (constraint.type == "allowed") {
        constraint.value = constraint_node.attribute("value").as_int();
    }
    else if (constraint.type == "between") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    }
    else if (constraint.type == "min") {
        constraint.min = constraint_node.attribute("value").as_int();
    }
    else if (constraint.type == "max") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all

    // Octet string constraints
    // lengthAllowed
    else if (constraint.type == "lengthBetween") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    }
    else if (constraint.type == "minLength") {
        constraint.min = constraint_node.attribute("value").as_int();
    }
    else if (constraint.type == "maxLength") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all

    // List constraints
    // countAllowed
    else if (constraint.type == "countBetween") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    }
    else if (constraint.type == "minCount") {
        constraint.min = constraint_node.attribute("value").as_int();
    }
    else if (constraint.type == "maxCount") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all
    // Entry constraint -> Child named "entry"
    // Character string constraints

    return 0;
}

int parse_conformance(const pugi::xml_node& conformance_node, std::optional<conformanceType>& optional_conformance)
{
    // TODO: These can be combined with logic operations to create complex expressions, should be caught
    // Mandatory conform
    if (!conformance_node.child("mandatoryConform").empty()) {
        conformanceType conformance;
        conformance.mandatory = true;
        optional_conformance = conformance;
    }
    // Optional conform
    else if (!conformance_node.child("optionalConform").empty()) {
        conformanceType conformance;
        conformance.optional = true;
        optional_conformance = conformance;
    }

    // Provisional conform
    else if (!conformance_node.child("provisionalConform").empty()) {
        conformanceType conformance;
        conformance.provisional = true;
        optional_conformance = conformance;
    }
    // Deprecated conform
    else if (!conformance_node.child("deprecatedConform").empty()) {
        conformanceType conformance;
        conformance.deprecated = true;
        optional_conformance = conformance;
    }
    // Disallowed conform
    else if (!conformance_node.child("disallowConform").empty()) {
        conformanceType conformance;
        conformance.disallowed = true;
        optional_conformance = conformance;
    }
    // In case no conformance is defined
    else {
        optional_conformance = std::nullopt;
    }

    // otherwiseConform

    return 0;
}

/*
 * Function used to
 */
int parse_access(const pugi::xml_node& access_node, accessType& access)
{
    if (!access_node.attribute("read").empty())
        access.read = access_node.attribute("read").as_bool();

    if (!access_node.attribute("write").empty())
        access.write = access_node.attribute("write").as_bool();

    if (!access_node.attribute("fabricScoped").empty())
        access.fabric_scoped = access_node.attribute("fabricScoped").as_bool();

    if (!access_node.attribute("fabricSensitive").empty())
        access.fabric_sensitive = access_node.attribute("fabricSensitive").as_bool();

    if (!access_node.attribute("readPrivilege").empty())
        access.read_privilege = access_node.attribute("readPrivilege").as_string();

    if (!access_node.attribute("writePrivilege").empty())
        access.write_privilege = access_node.attribute("writePrivilege").as_string();

    if (!access_node.attribute("invokePrivilege").empty())
        access.invoke_privilege = access_node.attribute("invokePrivilege").as_string();

    if (!access_node.attribute("timed").empty())
        access.timed = access_node.attribute("timed").as_bool();

    return 0;
}

int parse_common_data_quality()
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

        parse_conformance(bitfield_node.child("conformance"), bitfield.conformance);

        bitfields.push_back(bitfield);
    }

    return 0;
}

int parse_data_fields(const pugi::xml_node& data_fields_node, std::list<dataFieldType>& data_fields)
{
    // Iterate through all struct fields and parse them individually
    for (const auto& data_field_node : data_fields_node.children()) {
        dataFieldType data_field;
        data_field.id = data_field_node.attribute("id").as_int();
        data_field.name = data_field_node.attribute("name").value();

        if (!data_field_node.child("access").empty()) {
            accessType access;
            parse_access(data_field_node.child("access"), access);
            data_field.access = access;
        }

        parse_conformance(data_field_node, data_field.conformance);

        if (!data_field_node.attribute("summary").empty())
            data_field.summary = data_field_node.attribute("summary").value();

        if (!data_field_node.attribute("type").empty())
            data_field.type = data_field_node.attribute("type").value();

        if (!data_field_node.child("constraint").empty()) {
            constraintType constraint;
            parse_constraints(data_field_node.child("constraint"), constraint);
            data_field.constraint = constraint;
        }

        if (!data_field_node.child("quality").empty()) {
            otherQualityType quality;
            parse_other_qualities(data_field_node, quality);
            data_field.quality = quality;
        }

        //default
        data_fields.push_back(data_field);
    }

    return 0;
}

int parse_feature_map(const pugi::xml_node& feature_map_node, std::list<featureMapType>& featureMapList)
{
    // Iterate through all features and parse them individually
    for (const auto& feature : feature_map_node.children()) {
        featureMapType featureMap;
        featureMap.bit = feature.attribute("bit").as_int();
        parse_conformance(feature, featureMap.conformance);
        featureMap.code = feature.attribute("code").value();
        featureMap.name = feature.attribute("name").value();
        featureMap.summary = feature.attribute("summary").value();
        featureMapList.push_back(featureMap);
    }

    return 0;
}

int parse_event(const pugi::xml_node& event_node, eventType& event)
{
    event.id = event_node.attribute("id").as_int();
    event.name = event_node.attribute("name").value();

    parse_conformance(event_node, event.conformance);

    if (!event_node.child("access").empty()) {
        accessType access;
        parse_access(event_node.child("access"), access);
        event.access = access;
    }

    event.summary = event_node.attribute("summary").value();
    event.priority = event_node.attribute("priority").value();
    auto quality_node = event_node.child("quality");
    if (!quality_node.empty()) {
        otherQualityType otherQualities;
        parse_other_qualities(quality_node, otherQualities);
        event.quality = otherQualities;
    }

    parse_data_fields(event_node, event.data);

    return 0;
}

int parse_command(const pugi::xml_node& command_node, commandType& command)
{
    command.id = command_node.attribute("id").as_int();
    command.name = command_node.attribute("name").value();

    parse_conformance(command_node, command.conformance);

    if (!command_node.child("access").empty()) {
        accessType access;
        parse_access(command_node.child("access"), access);
        command.access = access;
    }

    command.summary = command_node.attribute("summary").value();
    // default
    command.direction = command_node.attribute("direction").value();
    command.response = command_node.attribute("response").value();

    return 0;
}

int parse_attribute(const pugi::xml_node& attribute_node, attributeType& attribute) {
    attribute.id = attribute_node.attribute("id").as_int();
    attribute.name = attribute_node.attribute("name").value();

    parse_conformance(attribute_node, attribute.conformance);

    if (!attribute_node.child("access").empty()) {
        accessType access;
        parse_access(attribute_node.child("access"), access);
        attribute.access = access;
    }

    attribute.summary = attribute_node.attribute("summary").value();
    attribute.type = attribute_node.attribute("type").value();
    auto quality_node = attribute_node.child("quality");
    if (!quality_node.empty()) {
        otherQualityType otherQualities;
        parse_other_qualities(quality_node, otherQualities);
        attribute.quality = otherQualities;
    }

    attribute.default_ = quality_node.attribute("default").value();

    return 0;

}

/*
 * Function used to parse globally defined custom data types.
 */
int parse_data_types(const pugi::xml_node& data_type_xml, clusterType& cluster)
{
    // Parse all data types based on enums.
    for (auto enum_node : data_type_xml.children("enum")) {
        std::list<enumItemType> enum_items;
        parse_enum_items(enum_node, enum_items);
        cluster.enums[enum_node.attribute("name").value()] = enum_items;
    }
    // Parse all data types based on bitmaps.
    for (auto bitmap_node : data_type_xml.children("bitmap")) {
        std::list<bitmapBitfieldType> bitfields;
        parse_bitfields(bitmap_node, bitfields);
        cluster.bitmaps[bitmap_node.attribute("name").value()] = bitfields;
    }

    return 0;
}

/*
 * Function used to parse classification information.
 */
int parse_cluster_classification(const pugi::xml_node& classification_xml, clusterClassificationType& classification)
{
    if (!classification_xml.attribute("hierarchy").empty())
        classification.hierarchy = classification_xml.attribute("hierarchy").value();

    if (!classification_xml.attribute("role").empty())
        classification.role = classification_xml.attribute("role").value();

    if (!classification_xml.attribute("picsCode").empty())
        classification.picsCode = classification_xml.attribute("picsCode").value();

    if (!classification_xml.attribute("scope").empty())
        classification.scope = classification_xml.attribute("scope").value();

    if (!classification_xml.attribute("baseCluster").empty())
        classification.baseCluster = classification_xml.attribute("baseCluster").value();

    if (!classification_xml.attribute("primaryTransaction").empty())
        classification.primaryTransaction = classification_xml.attribute("primaryTransaction").value();

    return 0;
}

/*
 * Function used to parse clusters.
 */
int parse_cluster(const pugi::xml_node& cluster_xml, clusterType& cluster)
{
    cluster.revision = cluster_xml.attribute("revision").as_int();

    // Iterate through all revisions and parse them individually
    for (const auto& revision_node : cluster_xml.child("revisionHistory").children()) {
        cluster.revision_history.insert({revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
    }

    // Parse the classification section
    if (!cluster_xml.child("classification").empty()) {
        clusterClassificationType classification;
        parse_cluster_classification(cluster_xml.child("classification"), classification);
        cluster.classification = classification;
    }

    // Parse the globally defined custom data types
    if (!cluster_xml.child("dataTypes").empty())
        parse_data_types(cluster_xml.child("dataTypes"), cluster);

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

    return 0;
}

int parse_device_classification(const pugi::xml_node& classification_node, deviceClassificationType& deviceClassification)
{
    if (!classification_node.attribute("superset").empty())
        deviceClassification.superset = classification_node.attribute("superset").value();

    if (!classification_node.attribute("class").empty())
        deviceClassification.class_ = classification_node.attribute("class").value();

    if (!classification_node.attribute("scope").empty())
        deviceClassification.scope = classification_node.attribute("scope").value();

    return 0;
}

/*
 * Function used to parse devices.
 */
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

    if (!device_xml.child("classification").empty()) {
        deviceClassificationType deviceClassification;
        parse_device_classification(device_xml.child("classification"), deviceClassification);
        device.classification = deviceClassification;
    }

    // Parse all data types and add them to a map
    auto data_type_node = device_xml.child("dataTypes");

    // Iterate through all clusters needed by the device and parse them individually
    for (const auto& cluster_node : device_xml.child("clusters").children("cluster")) {
        clusterType cluster;
        parse_cluster(cluster_node, cluster);
        device.clusters.push_back(cluster);
    }

    return 0;
}

int serialize_other_qualities(const otherQualityType& otherQualities, pugi::xml_node& quality_node)
{
    pugi::xml_node current_node = quality_node.append_child("quality");
    if (otherQualities.nullable.has_value())
        current_node.append_attribute("nullable").set_value(otherQualities.nullable.value());

    if (otherQualities.non_volatile.has_value()) {
        current_node.append_attribute("persistence").set_value("nonVolatile");
    } else if (otherQualities.fixed.has_value()) {
        current_node.append_attribute("persistence").set_value("fixed");
    } else {
        current_node.append_attribute("persistence").set_value("volatile");
    }

    if (otherQualities.scene.has_value())
        current_node.append_attribute("scene").set_value(otherQualities.scene.value());

    if (otherQualities.reportable.has_value())
        current_node.append_attribute("reportable").set_value(otherQualities.reportable.value());

    if (otherQualities.change_omitted.has_value())
        current_node.append_attribute("changeOmitted").set_value(otherQualities.change_omitted.value());

    if (otherQualities.singleton.has_value())
        current_node.append_attribute("singleton").set_value(otherQualities.singleton.value());

    if (otherQualities.diagnostics.has_value())
        current_node.append_attribute("diagnostics").set_value(otherQualities.diagnostics.value());

    if (otherQualities.large_message.has_value())
        current_node.append_attribute("largeMessage").set_value(otherQualities.large_message.value());

    if (otherQualities.quieter_reporting.has_value())
        current_node.append_attribute("quieterReporting").set_value(otherQualities.quieter_reporting.value());

    return 0;
}

int serialize_constraints(const constraintType& constraint, pugi::xml_node& constraint_node)
{
    // Constraint is defined in the description section
    if (constraint.type == "desc") {
        constraint_node.append_attribute("type").set_value("desc");
    }

    // Composed device type constraints
    // allowed
    // between
    // min
    // max

    // Numeric constraints
    else if (constraint.type == "allowed") {
        constraint_node.append_attribute("type").set_value("allowed");
        constraint_node.append_attribute("value").set_value(constraint.value.value());
    }
    else if (constraint.type == "between") {
        constraint_node.append_attribute("type").set_value("between");
        constraint_node.append_attribute("from").set_value(constraint.from.value());
        constraint_node.append_attribute("to").set_value(constraint.to.value());
    }
    else if (constraint.type == "min") {
        constraint_node.append_attribute("type").set_value("min");
        constraint_node.append_attribute("value").set_value(constraint.min.value());
    }
    else if (constraint.type == "max") {
        constraint_node.append_attribute("type").set_value("max");
        constraint_node.append_attribute("value").set_value(constraint.max.value());
    }
    // all

    // Octet string constraints
    // lengthAllowed
    else if (constraint.type == "lengthBetween") {
        constraint_node.append_attribute("type").set_value("lengthBetween");
        constraint_node.append_attribute("from").set_value(constraint.from.value());
        constraint_node.append_attribute("to").set_value(constraint.to.value());
    }
    else if (constraint.type == "minLength") {
        constraint_node.append_attribute("type").set_value("minLength");
        constraint_node.append_attribute("value").set_value(constraint.min.value());
    }
    else if (constraint.type == "maxLength") {
        constraint_node.append_attribute("type").set_value("maxLength");
        constraint_node.append_attribute("value").set_value(constraint.max.value());
    }
        // all

        // List constraints
        // countAllowed
    else if (constraint.type == "countBetween") {
        constraint_node.append_attribute("type").set_value("countBetween");
        constraint_node.append_attribute("from").set_value(constraint.from.value());
        constraint_node.append_attribute("to").set_value(constraint.to.value());
    }
    else if (constraint.type == "minCount") {
        constraint_node.append_attribute("type").set_value("minCount");
        constraint_node.append_attribute("value").set_value(constraint.min.value());
    }
    else if (constraint.type == "maxCount") {
        constraint_node.append_attribute("type").set_value("maxCount");
        constraint_node.append_attribute("value").set_value(constraint.max.value());
    }
    // all
    // Entry constraint -> Child named "entry"
    // Character string constraints

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

int serialize_access(const accessType& access, pugi::xml_node& access_node)
{
    if (access.read.has_value())
        access_node.append_attribute("read").set_value(access.read.value());

    if (access.write.has_value())
        access_node.append_attribute("write").set_value(access.write.value());

    if (access.fabric_scoped.has_value())
        access_node.append_attribute("fabricScoped").set_value(access.fabric_scoped.value());

    if (access.fabric_sensitive.has_value())
        access_node.append_attribute("fabricSensitive").set_value(access.fabric_sensitive.value());

    if (!access.read_privilege.empty())
        access_node.append_attribute("readPrivilege").set_value(access.read_privilege.c_str());

    if (!access.write_privilege.empty())
        access_node.append_attribute("writePrivilege").set_value(access.write_privilege.c_str());

    if (!access.invoke_privilege.empty())
        access_node.append_attribute("invokePrivilege").set_value(access.invoke_privilege.c_str());

    if (access.timed.has_value())
        access_node.append_attribute("timed").set_value(access.timed.value());

    return 0;
}

int serialize_event(const eventType& event, pugi::xml_node& event_xml)
{
    auto event_node = event_xml.append_child("event");
    return 0;
}

int serialize_command(const commandType& command, pugi::xml_node& command_xml)
{
    auto command_node = command_xml.append_child("command");

    command_node.append_attribute("id").set_value(decToHexa(command.id).c_str());
    command_node.append_attribute("name").set_value(command.name.c_str());

    if (command.conformance.has_value())
        serialize_conformance(command.conformance.value(), command_node);

    if (command.access.has_value()) {
        pugi::xml_node access_node = command_node.append_child("access");
        serialize_access(command.access.value(), access_node);
    }

    if (!command.summary.empty())
        command_node.append_attribute("summary").set_value(command.summary.c_str());

    //if (!command.default_.empty())
    //    command_node.append_attribute("default").set_value(command.default_.c_str());

    command_node.append_attribute("direction").set_value(command.direction.c_str());
    command_node.append_attribute("response").set_value(command.response.c_str());

    return 0;
}

int serialize_attribute(const attributeType& attribute, pugi::xml_node& attribute_xml)
{
    auto attribute_node = attribute_xml.append_child("attribute");

    attribute_node.append_attribute("id").set_value(decToHexa(attribute.id).c_str());
    attribute_node.append_attribute("name").set_value(attribute.name.c_str());

    if (attribute.conformance.has_value())
        serialize_conformance(attribute.conformance.value(), attribute_node);

    if (attribute.access.has_value()) {
        pugi::xml_node access_node = attribute_node.append_child("access");
        serialize_access(attribute.access.value(), access_node);
    }

    if (!attribute.summary.empty())
        attribute_node.append_attribute("summary").set_value(attribute.summary.c_str());

    if (attribute.quality.has_value())
        serialize_other_qualities(attribute.quality.value(), attribute_node);

    attribute_node.attribute("type").set_value(attribute.type.c_str());
    //attribute_node.attribute("default").set_value(attribute.default_.c_str());

    return 0;
}

int serialize_enum_items(const enumItemType& enumItem, pugi::xml_node& enum_item_node)
{
    enum_item_node.append_attribute("value").set_value(enumItem.value);
    enum_item_node.append_attribute("name").set_value(enumItem.name.c_str());
    enum_item_node.append_attribute("summary").set_value(enumItem.summary.c_str());
    if (enumItem.conformance.has_value())
        serialize_conformance(enumItem.conformance.value(), enum_item_node);

    return 0;
}

int serialize_bitfields(const bitmapBitfieldType& bitfield, pugi::xml_node& bitfield_node)
{
    bitfield_node.append_attribute("name").set_value(bitfield.name.c_str());
    bitfield_node.append_attribute("bit").set_value(bitfield.bit);
    bitfield_node.append_attribute("summary").set_value(bitfield.summary.c_str());
    if (bitfield.conformance.has_value())
        serialize_conformance(bitfield.conformance.value(), bitfield_node);

    return 0;
}

int serialize_data_types(const clusterType& cluster, pugi::xml_node& cluster_xml)
{
    auto data_type_node = cluster_xml.append_child("dataTypes");

    for (const auto& current_enum : cluster.enums) {
        pugi::xml_node enum_node = data_type_node.append_child("enum");
        enum_node.append_attribute("name").set_value(current_enum.first.c_str());
        for (const auto& enum_item : current_enum.second) {
            pugi::xml_node item_node = enum_node.append_child("item");
            serialize_enum_items(enum_item, item_node);
        }
    }

    for (const auto& bitmap : cluster.bitmaps) {
        pugi::xml_node bitmap_node = data_type_node.append_child("bitmap");
        bitmap_node.append_attribute("name").set_value(bitmap.first.c_str());
        for (const auto& bitfield : bitmap.second) {
            pugi::xml_node bitfield_node = bitmap_node.append_child("bitfield");
            serialize_bitfields(bitfield, bitfield_node);
        }
    }

    return 0;
}

int serialize_cluster_classification(const clusterClassificationType& classification, pugi::xml_node& classification_node)
{
    if (!classification.hierarchy.empty())
        classification_node.append_attribute("hierarchy").set_value(classification.hierarchy.c_str());

    if (!classification.role.empty())
        classification_node.append_attribute("role").set_value(classification.role.c_str());

    if (!classification.picsCode.empty())
        classification_node.append_attribute("picsCode").set_value(classification.picsCode.c_str());

    if (!classification.scope.empty())
        classification_node.append_attribute("scope").set_value(classification.scope.c_str());

    if (!classification.baseCluster.empty())
        classification_node.append_attribute("baseCluster").set_value(classification.baseCluster.c_str());

    if (!classification.primaryTransaction.empty())
        classification_node.append_attribute("primaryTransaction").set_value(classification.primaryTransaction.c_str());

    return 0;
}

int serialize_cluster(const clusterType& cluster, pugi::xml_node& cluster_xml)
{
    // Create the cluster node
    auto cluster_node = cluster_xml.append_child("cluster");

    cluster_node.append_attribute("id").set_value(decToHexa(cluster.id).c_str());
    cluster_node.append_attribute("name").set_value(cluster.name.c_str());

    if (cluster.conformance.has_value())
        serialize_conformance(cluster.conformance.value(), cluster_node);

    if (cluster.access.has_value()) {
        pugi::xml_node access_node = cluster_node.append_child("access");
        serialize_access(cluster.access.value(), access_node);
    }

    if (!cluster.summary.empty())
        cluster_node.append_attribute("summary").set_value(cluster.summary.c_str());

    // Serialize the classification information
    if (cluster.classification.has_value()) {
        pugi::xml_node classification_node = cluster_node.append_child("classification");
        serialize_cluster_classification(cluster.classification.value(), classification_node);
    }

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = cluster_node.append_child("revisionHistory");
    for (const auto& revision : cluster.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    // Serialize the custom data types
    serialize_data_types(cluster, cluster_node);

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

int serialize_device_classification(const deviceClassificationType& deviceClassification, pugi::xml_node& classification_node)
{
    if (!deviceClassification.superset.empty())
        classification_node.append_attribute("superset").set_value(deviceClassification.superset.c_str());

    if (!deviceClassification.class_.empty())
        classification_node.append_attribute("class").set_value(deviceClassification.class_.c_str());

    if (!deviceClassification.scope.empty())
        classification_node.append_attribute("scope").set_value(deviceClassification.scope.c_str());

    return 0;
}

int serialize_device(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml)
{
    auto device_node = device_xml.append_child("deviceType");

    device_node.append_attribute("id").set_value(decToHexa(device.id).c_str());
    device_node.append_attribute("name").set_value(device.name.c_str());
    device_node.append_attribute("revision").set_value(device.revision);
    if (!device.summary.empty())
        device_node.append_attribute("summary").set_value(device.summary.c_str());

    if (device.conformance.has_value())
        serialize_conformance(device.conformance.value(), device_node);

    if (device.access.has_value()) {
        pugi::xml_node access_node = device_node.append_child("access");
        serialize_access(device.access.value(), access_node);
    }

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = device_node.append_child("revisionHistory");
    for (const auto& revision : device.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    if (device.classification.has_value()) {
        pugi::xml_node classification_node = device_node.append_child("classification");
        serialize_device_classification(device.classification.value(), classification_node);
    }
    // Iterate through all clusters and serialize them individually
    auto clusters_node = device_node.append_child("clusters");
    for (const auto& cluster : device.clusters) {
        serialize_cluster(cluster, clusters_node);
    }

    return 0;
}
