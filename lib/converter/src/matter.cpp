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
#include <cstring>
#include <pugixml.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include "matter.h"

namespace matter {

//! Function used to parse the default value
DefaultType ParseDefaultType(const std::string& value)
{
    // Try to parse as int
    try {
        return std::stoi(value);
    } catch (...) {
        // Ignore the error
    }

    // Try to parse as double
    try {
        return std::stod(value);
    } catch (...) {
        // Ignore the error
    }

    // Try to parse as bool
    if (value == "true") {
        return true;
    } else if (value == "false") {
        return false;
    }

    // Try to parse as null
    if (value == "null")
        return std::nullopt;

    // Fallback to string
    return value;
}

//! Function used to parse a quality node into a OtherQuality object
OtherQuality ParseOtherQuality(const pugi::xml_node& parent_node) {
    OtherQuality other_quality;
    auto other_quality_node = parent_node.child("quality");
    if (!other_quality_node.attribute("nullable").empty())
        other_quality.nullable = other_quality_node.attribute("nullable").as_bool();

    if (!other_quality_node.attribute("persistence").empty()) {
        if (strcmp(other_quality_node.attribute("persistence").value(), "fixed") == 0) {
            other_quality.fixed = true;
        } else if (strcmp(other_quality_node.attribute("persistence").value(), "volatile") == 0) {
            other_quality.non_volatile = false;
        } else if (strcmp(other_quality_node.attribute("persistence").value(), "nonVolatile") == 0) {
            other_quality.non_volatile = true;
        }
    }
    if (!other_quality_node.attribute("scene").empty())
        other_quality.scene = other_quality_node.attribute("scene").as_bool();

    if (!other_quality_node.attribute("reportable").empty())
        other_quality.reportable = other_quality_node.attribute("reportable").as_bool();

    if (!other_quality_node.attribute("changeOmitted").empty())
        other_quality.change_omitted = other_quality_node.attribute("changeOmitted").as_bool();

    if (!other_quality_node.attribute("singleton").empty())
        other_quality.singleton = other_quality_node.attribute("singleton").as_bool();

    if (!other_quality_node.attribute("diagnostics").empty())
        other_quality.diagnostics = other_quality_node.attribute("diagnostics").as_bool();

    if (!other_quality_node.attribute("largeMessage").empty())
        other_quality.large_message = other_quality_node.attribute("largeMessage").as_bool();

    if (!other_quality_node.attribute("quieterReporting").empty())
        other_quality.quieter_reporting = other_quality_node.attribute("quieterReporting").as_bool();

    return other_quality;
}

//! Function used to parse a constraint node into a Constraint object
Constraint ParseConstraint(const pugi::xml_node& constraint_node) {
    Constraint constraint;
    constraint.type = constraint_node.attribute("type").value();

    // Constraint is defined in the description section
    if (constraint.type == "desc") {
        // In this case, there is nothing more to parse
    }
    // Numeric constraints
    else if (constraint.type == "allowed") {
        constraint.value = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "between") {
        constraint.min = constraint_node.attribute("from").as_int();
        constraint.max = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "min") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "max") {
        constraint.max = constraint_node.attribute("value").as_int();
    }

    // Octet string constraints
    else if (constraint.type == "lengthBetween") {
        constraint.min = constraint_node.attribute("from").as_int();
        constraint.max = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "minLength") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "maxLength") {
        constraint.max = constraint_node.attribute("value").as_int();
    }

    // List constraints
    else if (constraint.type == "countBetween") {
        constraint.min = constraint_node.attribute("from").as_int();
        constraint.max = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "minCount") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "maxCount") {
        constraint.max = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "entry") {
        constraint.entry_constraint_type = constraint_node.attribute("type").value();
    }
    // Character string constraints

    return constraint;
}

//! Function used to parse a logical term into a JSON representation
//! This will allow the easy exporting to the mapping as well as the
//! easy evaluation for the contained expression
nlohmann::json ParseLogicalTerm(const pugi::xml_node& logical_node) {
    nlohmann::json condition;
    std::string node_name = logical_node.name();
    if (node_name == "orTerm") {
        for (auto or_child: logical_node.children()) {
            condition["orTerm"].push_back(ParseLogicalTerm(or_child));
        }
    } else if (node_name == "andTerm") {
        for (auto and_child: logical_node.children()) {
            condition["andTerm"].push_back(ParseLogicalTerm(and_child));
        }
    } else if (node_name == "xorTerm") {
        for (auto xor_child: logical_node.children()) {
            condition["xorTerm"].push_back(ParseLogicalTerm(xor_child));
        }
    } else if (node_name == "notTerm") {
        for (auto not_child: logical_node.children()) {
            condition["notTerm"].push_back(ParseLogicalTerm(not_child));
        }
    }
    else if (node_name == "feature") {
        condition["feature"]["name"] = logical_node.attribute("name").value();
    } else if (node_name == "condition") {
        condition["condition"]["name"] = logical_node.attribute("name").value();
    } else if (node_name == "attribute") {
        condition["attribute"]["name"] = logical_node.attribute("name").value();
    }

    return condition;
}

//! Function used to parse a conformance node into a Conformance object
Conformance ParseConformance(const pugi::xml_node& conformance_node) {
    Conformance conformance;
    // Mandatory conform
    if (!conformance_node.child("mandatoryConform").empty()) {
        conformance.mandatory = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("mandatoryConform").children().empty())
            conformance.condition = ParseLogicalTerm(conformance_node.child("mandatoryConform").first_child());
    }
    // Optional conform
    else if (!conformance_node.child("optionalConform").empty()) {
        conformance.optional = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("optionalConform").children().empty())
            conformance.condition = ParseLogicalTerm(conformance_node.child("optionalConform").first_child());
    }

    // Provisional conform
    else if (!conformance_node.child("provisionalConform").empty()) {
        conformance.provisional = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("provisionalConform").children().empty())
            conformance.condition = ParseLogicalTerm(conformance_node.child("provisionalConform").first_child());
    }
    // Deprecated conform
    else if (!conformance_node.child("deprecateConform").empty()) {
        conformance.deprecated = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("deprecateConform").children().empty())
            conformance.condition = ParseLogicalTerm(conformance_node.child("deprecateConform").first_child());
    }
    // Disallowed conform
    else if (!conformance_node.child("disallowConform").empty()) {
        conformance.disallowed = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("disallowConform").children().empty())
            conformance.condition = ParseLogicalTerm(conformance_node.child("disallowConform").first_child());
    }
    // Otherwise conform
    else if (!conformance_node.child("otherwiseConform").empty()) {
        // Iterate through all child nodes
        for (const auto& otherwise_child : conformance_node.child("otherwiseConform").children()) {
            std::string child_name = otherwise_child.name();
            Conformance otherwise_conformance;
            if (child_name == "mandatoryConform") {
                otherwise_conformance.mandatory = true;
                if (!otherwise_child.children().empty())
                    otherwise_conformance.condition = ParseLogicalTerm(otherwise_child.first_child());
            }
            else if (child_name == "optionalConform") {
                otherwise_conformance.optional = true;
                if (!otherwise_child.children().empty())
                    otherwise_conformance.condition = ParseLogicalTerm(otherwise_child.first_child());
            }
            else if (child_name == "provisionalConform") {
                otherwise_conformance.provisional = true;
                if (!otherwise_child.children().empty())
                    otherwise_conformance.condition = ParseLogicalTerm(otherwise_child.first_child());
            }
            else if (child_name == "deprecateConform") {
                otherwise_conformance.deprecated = true;
                if (!otherwise_child.children().empty())
                    otherwise_conformance.condition = ParseLogicalTerm(otherwise_child.first_child());
            }
            else if (child_name == "disallowConform") {
                otherwise_conformance.disallowed = true;
                if (!otherwise_child.children().empty())
                    otherwise_conformance.condition = ParseLogicalTerm(otherwise_child.first_child());
            }
            // Recursively process the child nodes
            conformance.otherwise.push_back(otherwise_conformance);
        }
    }

    return conformance;
}

//! Function used to parse a access node into a Access object
Access ParseAccess(const pugi::xml_node& access_node) {
    Access access;
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

    return access;
}

//! Function used to parse a enum item
Item ParseItem(const pugi::xml_node& enum_item_node) {
    Item item;

    item.value = enum_item_node.attribute("value").as_int();
    item.name = enum_item_node.attribute("name").value();
    item.summary = FilterMultipleSpaces(enum_item_node.attribute("summary").value());
    item.conformance = ParseConformance(enum_item_node);

    return item;
}

//! Function used to parse a bitmap bitfield
Bitfield ParseBitfield(const pugi::xml_node& bitfield_node) {
    Bitfield bitfield;

    bitfield.bit = bitfield_node.attribute("bit").as_int();
    bitfield.name = bitfield_node.attribute("name").value();
    bitfield.summary = FilterMultipleSpaces(bitfield_node.attribute("summary").value());
    bitfield.conformance = ParseConformance(bitfield_node);

    return bitfield;
}

//! Function used to parse a data field
DataField ParseDataField(const pugi::xml_node& data_field_node) {
    DataField data_field;

    data_field.id = data_field_node.attribute("id").as_int();
    data_field.name = data_field_node.attribute("name").value();

    if (!data_field_node.child("access").empty())
        data_field.access = ParseAccess(data_field_node.child("access"));

    data_field.conformance = ParseConformance(data_field_node);

    if (!data_field_node.attribute("summary").empty())
        data_field.summary = FilterMultipleSpaces(data_field_node.attribute("summary").value());

    if (!data_field_node.attribute("type").empty())
        data_field.type = data_field_node.attribute("type").value();

    if (!data_field_node.child("constraint").empty())
        data_field.constraint = ParseConstraint(data_field_node.child("constraint"));

    if (!data_field_node.child("entry").empty())
        data_field.constraint = ParseConstraint(data_field_node.child("entry"));

    if (!data_field_node.child("quality").empty())
        data_field.quality = ParseOtherQuality(data_field_node);

    if (!data_field_node.attribute("default").empty())
        data_field.default_ = ParseDefaultType(data_field_node.attribute("default").value());

    return data_field;
}

//! Function used to parse a feature
Feature ParseFeature(const pugi::xml_node& feature_node) {
    Feature feature;

    feature.bit = feature_node.attribute("bit").as_int();
    feature.conformance = ParseConformance(feature_node);
    feature.code = feature_node.attribute("code").value();
    feature.name = feature_node.attribute("name").value();
    feature.summary = FilterMultipleSpaces(feature_node.attribute("summary").value());

    return feature;
}

//! Function used to parse a Matter event
Event ParseEvent(const pugi::xml_node& event_node) {
    Event event;
    event.id = event_node.attribute("id").as_int();
    event.name = event_node.attribute("name").value();

    event.conformance = ParseConformance(event_node);

    if (!event_node.child("access").empty())
        event.access = ParseAccess(event_node.child("access"));

    event.summary = FilterMultipleSpaces(event_node.attribute("summary").value());
    event.priority = event_node.attribute("priority").value();
    if (!event_node.child("quality").empty())
        event.quality = ParseOtherQuality(event_node);

    for (const auto& field_node : event_node.children("field")) {
        event.data.push_back(ParseDataField(field_node));
    }

    return event;
}

//! Function used to parse a Matter command
Command ParseCommand(const pugi::xml_node& command_node) {
    Command command;
    command.id = command_node.attribute("id").as_int();
    command.name = command_node.attribute("name").value();

    command.conformance = ParseConformance(command_node);

    if (!command_node.child("access").empty())
        command.access = ParseAccess(command_node.child("access"));

    command.summary = FilterMultipleSpaces(command_node.attribute("summary").value());
    if (!command_node.attribute("default").empty())
        command.default_ = ParseDefaultType(command_node.attribute("default").value());
    command.direction = command_node.attribute("direction").value();
    command.response = command_node.attribute("response").value();

    for (const auto& field_node : command_node.children("field")) {
        command.command_fields.push_back(ParseDataField(field_node));
    }

    return command;
}

//! Function used to parse a Matter attribute
Attribute ParseAttribute(const pugi::xml_node& attribute_node) {
    Attribute attribute;
    attribute.id = attribute_node.attribute("id").as_int();
    attribute.name = attribute_node.attribute("name").value();

    attribute.conformance = ParseConformance(attribute_node);

    if (!attribute_node.child("access").empty())
        attribute.access = ParseAccess(attribute_node.child("access"));

    attribute.summary = FilterMultipleSpaces(attribute_node.attribute("summary").value());
    attribute.type = attribute_node.attribute("type").value();

    if (!attribute_node.child("constraint").empty())
        attribute.constraint = ParseConstraint(attribute_node.child("constraint"));

    if (!attribute_node.child("quality").empty())
        attribute.quality = ParseOtherQuality(attribute_node);

    if (!attribute_node.attribute("default").empty())
        attribute.default_ = ParseDefaultType(attribute_node.attribute("default").value());

    return attribute;

}

//! Function used to parse globally defined custom data types.
void ParseDataTypes(const pugi::xml_node& data_type_xml, Cluster& cluster) {
    // Parse all data types based on enums.
    for (const auto& struct_node: data_type_xml.children("struct")) {
        std::list<DataField> struct_fields;
        for (const auto& field_node : struct_node.children("field")) {
            struct_fields.push_back(ParseDataField(field_node));
        }
        cluster.structs[struct_node.attribute("name").value()] = struct_fields;
    }

    // Parse all data types based on enums.
    for (const auto& enum_node: data_type_xml.children("enum")) {
        std::list<Item> enum_items;
        for (const auto& item_node : enum_node.children("item")) {
            enum_items.push_back(ParseItem(item_node));
        }
        cluster.enums[enum_node.attribute("name").value()] = enum_items;
    }

    // Parse all data types based on bitmaps.
    for (const auto& bitmap_node: data_type_xml.children("bitmap")) {
        std::list<Bitfield> bitfields;
        for (const auto& bitfield_node : bitmap_node.children("bitfield")) {
            bitfields.push_back(ParseBitfield(bitfield_node));
        }
        cluster.bitmaps[bitmap_node.attribute("name").value()] = bitfields;
    }
}

//! Function used to parse classification information.
ClusterClassification ParseClusterClassification(const pugi::xml_node& classification_xml) {
    ClusterClassification cluster_classification;
    if (!classification_xml.attribute("hierarchy").empty())
        cluster_classification.hierarchy = classification_xml.attribute("hierarchy").value();

    if (!classification_xml.attribute("role").empty())
        cluster_classification.role = classification_xml.attribute("role").value();

    if (!classification_xml.attribute("picsCode").empty())
        cluster_classification.pics_code = classification_xml.attribute("picsCode").value();

    if (!classification_xml.attribute("scope").empty())
        cluster_classification.scope = classification_xml.attribute("scope").value();

    if (!classification_xml.attribute("baseCluster").empty())
        cluster_classification.base_cluster = classification_xml.attribute("baseCluster").value();

    if (!classification_xml.attribute("primaryTransaction").empty())
        cluster_classification.primary_transaction = classification_xml.attribute("primaryTransaction").value();

    return cluster_classification;
}

//! Function used to parse clusters.
Cluster ParseCluster(const pugi::xml_node& cluster_xml) {
    Cluster cluster;
    cluster.id = cluster_xml.attribute("id").as_int();
    cluster.name = cluster_xml.attribute("name").value();
    cluster.conformance = ParseConformance(cluster_xml);
    cluster.summary = FilterMultipleSpaces(cluster_xml.attribute("summary").value());
    if (!cluster_xml.attribute("side").empty())
        cluster.side = cluster_xml.attribute("side").value();
    cluster.revision = cluster_xml.attribute("revision").as_int();

    // Iterate through all revisions and parse them individually
    for (const auto &revision_node: cluster_xml.child("revisionHistory").children()) {
        cluster.revision_history.insert(
                {revision_node.attribute("revision").as_int(), FilterMultipleSpaces(revision_node.attribute("summary").value())});
    }

    // Iterate through all cluster aliases and parse them individually
    if (!cluster_xml.child("clusterIds").empty()) {
        for (const auto& cluster_alias_node : cluster_xml.child("clusterIds").children()) {
            std::pair<uint32_t, std::string> cluster_alias;
            cluster_alias.first = cluster_alias_node.attribute("id").as_uint();
            cluster_alias.second = cluster_alias_node.attribute("name").value();
            cluster.cluster_aliases.push_back(cluster_alias);
        }
    }

    // Parse the classification section
    if (!cluster_xml.child("classification").empty())
        cluster.classification = ParseClusterClassification(cluster_xml.child("classification"));

    // Parse the feature map
    if (!cluster_xml.child("features").empty()) {
        for (const auto& feature : cluster_xml.child("features").children()) {
            cluster.feature_map.push_back(ParseFeature(feature));
        }
    }

    // Parse the globally defined custom data types
    if (!cluster_xml.child("dataTypes").empty())
        ParseDataTypes(cluster_xml.child("dataTypes"), cluster);

    // Iterate through all attributes and parse them individually
    for (const auto &attribute_node: cluster_xml.child("attributes").children()) {
        cluster.attributes.push_back(ParseAttribute(attribute_node));
    }

    // Iterate through all commands and parse them individually
    for (const auto &command_node: cluster_xml.child("commands").children()) {
        // Split the commands into client and server commands
        std::string direction = command_node.attribute("direction").value();
        if (direction == "commandToServer") {
            cluster.client_commands.push_back(ParseCommand(command_node));
        } else {
            Command server_command = ParseCommand(command_node);
            cluster.server_commands[server_command.name] = server_command;
        }
    }

    // Iterate through all events and parse them individually
    for (const auto &event_node: cluster_xml.child("events").children()) {
        cluster.events.push_back(ParseEvent(event_node));
    }

    return cluster;
}

//! Function used to parse a device type classification
DeviceClassification ParseDeviceClassification(const pugi::xml_node& classification_node)
{
    DeviceClassification device_classification;
    if (!classification_node.attribute("superset").empty())
        device_classification.superset = classification_node.attribute("superset").value();

    if (!classification_node.attribute("class").empty())
        device_classification.class_ = classification_node.attribute("class").value();

    if (!classification_node.attribute("scope").empty())
        device_classification.scope = classification_node.attribute("scope").value();

    return device_classification;
}

//! Function used to parse a device type definition.
Device ParseDevice(const pugi::xml_node& device_xml) {
    Device device;
    device.id = device_xml.attribute("id").as_int();
    device.name = device_xml.attribute("name").value();
    device.revision = device_xml.attribute("revision").as_int();

    // Iterate through all revisions and parse them individually
    for (const auto &revision_node: device_xml.child("revisionHistory").children()) {
        device.revision_history.insert(
                {revision_node.attribute("revision").as_int(), FilterMultipleSpaces(revision_node.attribute("summary").value())});
    }

    if (!device_xml.child("classification").empty())
        device.classification =  ParseDeviceClassification(device_xml.child("classification"));

    // Parse all data types and add them to a map
    auto data_type_node = device_xml.child("dataTypes");

    // Iterate through all clusters needed by the device and parse them individually
    for (const auto &cluster_node: device_xml.child("clusters").children("cluster")) {
        device.clusters.push_back(ParseCluster(cluster_node));
    }

    return device;
}

//! Serializes a other quality object into a xml node and appends it to the parent node
void SerializeOtherQuality(const OtherQuality& other_quality, pugi::xml_node& parent_node) {
    pugi::xml_node quality_node = parent_node.append_child("quality");
    if (other_quality.nullable.has_value())
        quality_node.append_attribute("nullable").set_value(other_quality.nullable.value());

    if (other_quality.non_volatile.has_value()) {
        if (other_quality.non_volatile.value())
            quality_node.append_attribute("persistence").set_value("nonVolatile");
        else
            quality_node.append_attribute("persistence").set_value("volatile");
    }
    else if (other_quality.fixed.has_value()) {
        quality_node.append_attribute("persistence").set_value("fixed");
    }

    if (other_quality.scene.has_value())
        quality_node.append_attribute("scene").set_value(other_quality.scene.value());

    if (other_quality.reportable.has_value())
        quality_node.append_attribute("reportable").set_value(other_quality.reportable.value());

    if (other_quality.change_omitted.has_value())
        quality_node.append_attribute("changeOmitted").set_value(other_quality.change_omitted.value());

    if (other_quality.singleton.has_value())
        quality_node.append_attribute("singleton").set_value(other_quality.singleton.value());

    if (other_quality.diagnostics.has_value())
        quality_node.append_attribute("diagnostics").set_value(other_quality.diagnostics.value());

    if (other_quality.large_message.has_value())
        quality_node.append_attribute("largeMessage").set_value(other_quality.large_message.value());

    if (other_quality.quieter_reporting.has_value())
        quality_node.append_attribute("quieterReporting").set_value(other_quality.quieter_reporting.value());
}

//! Serializes the given default value into its actual contained datatype
void SerializeDefaultType(const DefaultType& value, const char* attribute_name, pugi::xml_node& target_node) {
    if (std::holds_alternative<double>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<double>(value));

    else if (std::holds_alternative<int64_t>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<int64_t>(value));

    else if (std::holds_alternative<uint64_t>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<uint64_t>(value));

    else if (std::holds_alternative<std::string>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<std::string>(value).c_str());

    else if (std::holds_alternative<bool>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<bool>(value));

    else if (std::holds_alternative<std::optional<std::monostate>>(value))
        target_node.append_attribute(attribute_name).set_value("null");
}

//! Serializes the given numeric value into its actual contained datatype
void SerializeNumericType(const NumericType& value, const char* attribute_name, pugi::xml_node& target_node) {
    if (std::holds_alternative<double>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<double>(value));

    else if (std::holds_alternative<int64_t>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<int64_t>(value));

    else if (std::holds_alternative<uint64_t>(value))
        target_node.append_attribute(attribute_name).set_value(std::get<uint64_t>(value));
}

//! Serializes a constraint object into a xml node and appends it to the given parent node
void SerializeConstraint(const Constraint& constraint, pugi::xml_node& parent_node) {
    // If the constraint is an entry constraint
    if (constraint.type == "entry") {
        auto constraint_node = parent_node.append_child("entry");
        constraint_node.append_attribute("type").set_value(constraint.entry_constraint_type.c_str());
    }
    else if (!constraint.type.empty()) {
        auto constraint_node = parent_node.append_child("constraint");
        // Constraint is defined in the description section
        if (constraint.type == "desc")
            constraint_node.append_attribute("type").set_value("desc");

        // Numeric constraints
        else if (constraint.type == "allowed") {
            constraint_node.append_attribute("type").set_value("allowed");
            SerializeDefaultType(constraint.value.value(), "value", constraint_node);
        } else if (constraint.type == "between") {
            constraint_node.append_attribute("type").set_value("between");
            SerializeNumericType(constraint.min.value(), "from", constraint_node);
            SerializeNumericType(constraint.max.value(), "to", constraint_node);
        } else if (constraint.type == "min") {
            constraint_node.append_attribute("type").set_value("min");
            SerializeNumericType(constraint.min.value(), "value", constraint_node);
        } else if (constraint.type == "max") {
            constraint_node.append_attribute("type").set_value("max");
            SerializeNumericType(constraint.max.value(), "value", constraint_node);
        }

        // Octet string constraints
        else if (constraint.type == "lengthBetween") {
            constraint_node.append_attribute("type").set_value("lengthBetween");
            SerializeNumericType(constraint.min.value(), "from", constraint_node);
            SerializeNumericType(constraint.max.value(), "to", constraint_node);
        } else if (constraint.type == "minLength") {
            constraint_node.append_attribute("type").set_value("minLength");
            SerializeNumericType(constraint.min.value(), "value", constraint_node);
        } else if (constraint.type == "maxLength") {
            constraint_node.append_attribute("type").set_value("maxLength");
            SerializeNumericType(constraint.max.value(), "value", constraint_node);
        }

        // List constraints
        else if (constraint.type == "countBetween") {
            constraint_node.append_attribute("type").set_value("countBetween");
            SerializeNumericType(constraint.min.value(), "from", constraint_node);
            SerializeNumericType(constraint.max.value(), "to", constraint_node);
        } else if (constraint.type == "minCount") {
            constraint_node.append_attribute("type").set_value("minCount");

            SerializeNumericType(constraint.min.value(), "value", constraint_node);
        } else if (constraint.type == "maxCount") {
            constraint_node.append_attribute("type").set_value("maxCount");
            SerializeNumericType(constraint.max.value(), "value", constraint_node);
        }
        // Character string constraints
    }
}

//! Serialize a matter conformance logical term into nested xml nodes
//! This function is usually used in combination with SerializeConformance
void SerializeLogicalTerm(const nlohmann::json& condition, pugi::xml_node& parent_node)
{
    if (condition.contains("orTerm")) {
        auto or_node = parent_node.append_child("orTerm");
        for (const auto& or_child : condition.at("orTerm")) {
            SerializeLogicalTerm(or_child, or_node);
        }
    }

    else if (condition.contains("andTerm")) {
        auto and_node = parent_node.append_child("andTerm");
        for (const auto& and_child : condition.at("andTerm")) {
            SerializeLogicalTerm(and_child, and_node);
        }
    }

    else if (condition.contains("xorTerm")) {
        auto xor_node = parent_node.append_child("xorTerm");
        for (const auto& xor_child : condition.at("xorTerm")) {
            SerializeLogicalTerm(xor_child, xor_node);
        }
    }

    else if (condition.contains("notTerm")) {
        auto not_node = parent_node.append_child("notTerm");
        for (const auto& not_child : condition.at("notTerm")) {
            SerializeLogicalTerm(not_child, not_node);
        }
    }
    else if (condition.contains("feature")) {
        std::string name;
        condition.at("feature").at("name").get_to(name);
        parent_node.append_child("feature").append_attribute("name").set_value(name.c_str());
    }
    else if (condition.contains("condition")) {
        std::string name;
        condition.at("condition").at("name").get_to(name);
        parent_node.append_child("condition").append_attribute("name").set_value(name.c_str());
    }
    else if (condition.contains("attribute")) {
        std::string name;
        condition.at("attribute").at("name").get_to(name);
        parent_node.append_child("attribute").append_attribute("name").set_value(name.c_str());
    }
}

//! Serializes a conformance object into a xml node and appends it to the given parent node
void SerializeConformance(const Conformance& conformance, pugi::xml_node& parent_node) {
    pugi::xml_node conformance_node;
    if (conformance.mandatory)
        conformance_node = parent_node.append_child("mandatoryConform");
    else if (conformance.optional)
        conformance_node = parent_node.append_child("optionalConform");
    else if (conformance.provisional)
        conformance_node = parent_node.append_child("provisionalConform");
    else if (conformance.deprecated)
        conformance_node = parent_node.append_child("deprecateConform");
    else if (conformance.disallowed)
        conformance_node = parent_node.append_child("disallowConform");
    else if (!conformance.otherwise.empty()) {
        conformance_node = parent_node.append_child("otherwiseConform");
        for (const auto& otherwise_conformance : conformance.otherwise) {
            SerializeConformance(otherwise_conformance, conformance_node);
        }
    }
    if (!conformance.condition.is_null())
        SerializeLogicalTerm(conformance.condition, conformance_node);
}

//! Serializes a access object into a xml node and appends it to the given parent node
void SerializeAccess(const Access& access, pugi::xml_node& parent_node) {
    auto access_node = parent_node.append_child("access");
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
}

//! Serializes a data field object into a xml node and appends it to the given parent node
void SerializeDataField(const DataField& data_field, pugi::xml_node& parent_node) {
    auto data_field_node = parent_node.append_child("field");
    data_field_node.append_attribute("id").set_value(data_field.id);
    data_field_node.append_attribute("name").set_value(data_field.name.c_str());

    if (data_field.conformance.has_value())
        SerializeConformance(data_field.conformance.value(), data_field_node);

    if (data_field.access.has_value())
        SerializeAccess(data_field.access.value(), data_field_node);

    if (!data_field.summary.empty())
        data_field_node.append_attribute("summary").set_value(data_field.summary.c_str());

    if (!data_field.type.empty())
        data_field_node.append_attribute("type").set_value(data_field.type.c_str());

    if (data_field.constraint.has_value())
        SerializeConstraint(data_field.constraint.value(), data_field_node);

    if (data_field.quality.has_value())
        SerializeOtherQuality(data_field.quality.value(), data_field_node);

    if (data_field.default_.has_value())
        SerializeDefaultType(data_field.default_.value(), "default", data_field_node);
}

//! Serializes a event object into a xml node and appends it to the given parent node
void SerializeEvent(const Event& event, pugi::xml_node& events_node) {
    auto event_node = events_node.append_child("event");

    event_node.append_attribute("id").set_value(IntToHex(event.id).c_str());
    event_node.append_attribute("name").set_value(event.name.c_str());
    if (event.conformance.has_value())
        SerializeConformance(event.conformance.value(), event_node);
    if (event.access.has_value())
        SerializeAccess(event.access.value(), event_node);
    if (!event.summary.empty())
        event_node.append_attribute("summary").set_value(event.summary.c_str());
    event_node.append_attribute("priority").set_value(event.priority.c_str());
    if (event.quality.has_value())
        SerializeOtherQuality(event.quality.value(), event_node);
    for (const auto& data_field : event.data) {
        SerializeDataField(data_field, event_node);
    }
}

//! Serializes a command object and into a xml node and appends it to the given parent node
void SerializeCommand(const Command& command, pugi::xml_node& commands_node) {
    auto command_node = commands_node.append_child("command");
    command_node.append_attribute("id").set_value(IntToHex(command.id).c_str());
    command_node.append_attribute("name").set_value(command.name.c_str());

    if (!command.summary.empty())
        command_node.append_attribute("summary").set_value(command.summary.c_str());

    if (command.default_.has_value())
        SerializeDefaultType(command.default_.value(), "default", command_node);

    command_node.append_attribute("direction").set_value(command.direction.c_str());

    if (!command.response.empty())
        command_node.append_attribute("response").set_value(command.response.c_str());

    if (command.access.has_value())
        SerializeAccess(command.access.value(), command_node);

    if (command.conformance.has_value())
       SerializeConformance(command.conformance.value(), command_node);

    // Serialize the command fields
    for (const auto &field: command.command_fields) {
        SerializeDataField(field, command_node);
    }
}

//! Serialize a attribute object into a xml node and appends it to the given parent node
void SerializeAttribute(const Attribute& attribute, pugi::xml_node& attributes_node) {
    auto attribute_node = attributes_node.append_child("attribute");

    attribute_node.append_attribute("id").set_value(IntToHex(attribute.id).c_str());
    attribute_node.append_attribute("name").set_value(attribute.name.c_str());

    if (!attribute.summary.empty())
        attribute_node.append_attribute("summary").set_value(attribute.summary.c_str());

    if (!attribute.type.empty())
        attribute_node.append_attribute("type").set_value(attribute.type.c_str());

    if (attribute.default_.has_value())
        SerializeDefaultType(attribute.default_.value(), "default", attribute_node);

    if (attribute.access.has_value())
        SerializeAccess(attribute.access.value(), attribute_node);

    if (attribute.quality.has_value())
        SerializeOtherQuality(attribute.quality.value(), attribute_node);

    if (attribute.conformance.has_value())
        SerializeConformance(attribute.conformance.value(), attribute_node);

    if (attribute.constraint.has_value())
        SerializeConstraint(attribute.constraint.value(), attribute_node);
}

//! Serialize a enum item into a xml node
void SerializeItem(const Item& item, pugi::xml_node& enum_node) {
    auto item_node = enum_node.append_child("item");
    item_node.append_attribute("value").set_value(item.value);
    item_node.append_attribute("name").set_value(item.name.c_str());
    item_node.append_attribute("summary").set_value(item.summary.c_str());
    if (item.conformance.has_value())
        SerializeConformance(item.conformance.value(), item_node);
}

//! Serialize a bitfield into a xml node
void SerializeBitfield(const Bitfield& bitfield, pugi::xml_node& bitmap_node) {
    auto bitfield_node = bitmap_node.append_child("bitfield");
    bitfield_node.append_attribute("name").set_value(bitfield.name.c_str());
    bitfield_node.append_attribute("bit").set_value(bitfield.bit);
    bitfield_node.append_attribute("summary").set_value(bitfield.summary.c_str());
    if (bitfield.conformance.has_value())
        SerializeConformance(bitfield.conformance.value(), bitfield_node);
}

//! Serializes the dataType section of the given cluster object into xml nodes and appends them to the given parent node
void SerializeDataTypes(const Cluster& cluster, pugi::xml_node& cluster_xml) {
    auto data_type_node = cluster_xml.append_child("dataTypes");

    // number

    for (const auto& current_struct : cluster.structs) {
        pugi::xml_node struct_node = data_type_node.append_child("struct");
        struct_node.append_attribute("name").set_value(current_struct.first.c_str());
        for (const auto& struct_field : current_struct.second) {
            SerializeDataField(struct_field, struct_node);
        }
    }

    for (const auto& current_enum: cluster.enums) {
        pugi::xml_node enum_node = data_type_node.append_child("enum");
        enum_node.append_attribute("name").set_value(current_enum.first.c_str());
        for (const auto &enum_item: current_enum.second) {
            SerializeItem(enum_item, enum_node);
        }
    }

    for (const auto& bitmap: cluster.bitmaps) {
        pugi::xml_node bitmap_node = data_type_node.append_child("bitmap");
        bitmap_node.append_attribute("name").set_value(bitmap.first.c_str());
        for (const auto &bitfield: bitmap.second) {
            SerializeBitfield(bitfield, bitmap_node);
        }
    }
}

//! Serialize a feature map into a list of xml nodes
void SerializeFeatureMap(const std::list<matter::Feature>& features_map, pugi::xml_node& cluster_node)
{
    auto features_node = cluster_node.append_child("features");
    for (const auto& feature : features_map) {
        auto feature_node = features_node.append_child("feature");
        feature_node.append_attribute("bit").set_value(feature.bit);
        feature_node.append_attribute("code").set_value(feature.code.c_str());
        feature_node.append_attribute("name").set_value(feature.name.c_str());
        feature_node.append_attribute("summary").set_value(feature.summary.c_str());
        if (feature.conformance.has_value())
            SerializeConformance(feature.conformance.value(), feature_node);
    }
}

//! Serializes a cluster classification into a xml node and appends it to the given parent node
void SerializeClusterClassification(const ClusterClassification& cluster_classification, pugi::xml_node& cluster_node) {
    auto classification_node = cluster_node.append_child("classification");
    if (!cluster_classification.hierarchy.empty())
        classification_node.append_attribute("hierarchy").set_value(cluster_classification.hierarchy.c_str());

    if (!cluster_classification.role.empty())
        classification_node.append_attribute("role").set_value(cluster_classification.role.c_str());

    if (!cluster_classification.pics_code.empty())
        classification_node.append_attribute("picsCode").set_value(cluster_classification.pics_code.c_str());

    if (!cluster_classification.scope.empty())
        classification_node.append_attribute("scope").set_value(cluster_classification.scope.c_str());

    if (!cluster_classification.base_cluster.empty())
        classification_node.append_attribute("baseCluster").set_value(cluster_classification.base_cluster.c_str());

    if (!cluster_classification.primary_transaction.empty())
        classification_node.append_attribute("primary_transaction").set_value(
                cluster_classification.primary_transaction.c_str());
}

//! Serializes a cluster object into a xml document
void SerializeCluster(const Cluster &cluster, pugi::xml_document& cluster_xml)
{
    // Create the cluster node
    auto cluster_node = cluster_xml.append_child("cluster");
    cluster_node.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
    cluster_node.append_attribute("xsi:schemaLocation").set_value("types types.xsd cluster cluster.xsd");

    cluster_node.append_attribute("id").set_value(IntToHex(cluster.id).c_str());
    cluster_node.append_attribute("name").set_value(cluster.name.c_str());
    cluster_node.append_attribute("revision").set_value(cluster.revision);

    if (cluster.access.has_value())
        SerializeAccess(cluster.access.value(), cluster_node);

    if (!cluster.summary.empty())
        cluster_node.append_attribute("summary").set_value(cluster.summary.c_str());

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = cluster_node.append_child("revisionHistory");
    for (const auto &revision: cluster.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    // Iterate through all cluster aliases and serialize them individually
    auto cluster_aliases_node = cluster_node.append_child("clusterIds");
    for (const auto& cluster_alias : cluster.cluster_aliases) {
        auto cluster_alias_node = cluster_aliases_node.append_child("clusterId");
        cluster_alias_node.append_attribute("id").set_value(IntToHex(cluster_alias.first).c_str());
        cluster_alias_node.append_attribute("name").set_value(cluster_alias.second.c_str());
    }

    // Serialize the classification information
    if (cluster.classification.has_value())
        SerializeClusterClassification(cluster.classification.value(), cluster_node);

    if (!cluster.feature_map.empty())
        SerializeFeatureMap(cluster.feature_map, cluster_node);

    // Serialize the custom data types
    SerializeDataTypes(cluster, cluster_node);

    if (!cluster.attributes.empty()) {
        // Iterate through all attributes and serialize them individually
        auto attributes_node = cluster_node.append_child("attributes");
        for (const auto &attribute: cluster.attributes) {
            SerializeAttribute(attribute, attributes_node);
        }
    }

    if (!cluster.client_commands.empty()) {
        // Iterate through all client and server commands and serialize them individually
        auto commands_node = cluster_node.append_child("commands");
        for (const auto& client_command : cluster.client_commands) {
            SerializeCommand(client_command, commands_node);
        }
        for (const auto& server_command : cluster.server_commands) {
            SerializeCommand(server_command.second, commands_node);
        }
    }

    if (!cluster.events.empty()) {
        // Iterate through all events and serialize them individually
        auto events_node = cluster_node.append_child("events");
        for (const auto &event: cluster.events) {
            SerializeEvent(event, events_node);
        }
    }
}

//! Serializes a device type classification object into a xml node and appends it to the given parent node
void SerializeDeviceClassification(const DeviceClassification& device_classification, pugi::xml_node& device_node) {
    auto classification_node = device_node.append_child("classification");
    if (!device_classification.superset.empty())
        classification_node.append_attribute("superset").set_value(device_classification.superset.c_str());

    if (!device_classification.class_.empty())
        classification_node.append_attribute("class").set_value(device_classification.class_.c_str());

    if (!device_classification.scope.empty())
        classification_node.append_attribute("scope").set_value(device_classification.scope.c_str());
}

//! Serializes a device object into a xml document
void SerializeDevice(const Device& device, pugi::xml_document& device_xml)
{
    auto device_node = device_xml.append_child("deviceType");
    device_node.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
    device_node.append_attribute("xsi:schemaLocation").set_value("types types.xsd devicetype devicetype.xsd");
    device_node.append_attribute("id").set_value(IntToHex(device.id).c_str());
    device_node.append_attribute("name").set_value(device.name.c_str());
    device_node.append_attribute("revision").set_value(device.revision);
    if (!device.summary.empty())
        device_node.append_attribute("summary").set_value(device.summary.c_str());

    if (device.conformance.has_value())
        SerializeConformance(device.conformance.value(), device_node);

    if (device.access.has_value())
        SerializeAccess(device.access.value(), device_node);

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = device_node.append_child("revisionHistory");
    for (const auto &revision: device.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    if (device.classification.has_value())
        SerializeDeviceClassification(device.classification.value(), device_node);

    device_node.append_child("conditions");
    // Iterate through all clusters and serialize them individually
    auto clusters_node = device_node.append_child("clusters");
    for (const auto &cluster: device.clusters) {
        auto cluster_node = clusters_node.append_child("cluster");
        cluster_node.append_attribute("id").set_value(IntToHex(cluster.id).c_str());
        cluster_node.append_attribute("name").set_value(cluster.name.c_str());
        if (!cluster.side.empty())
            cluster_node.append_attribute("side").set_value(cluster.side.c_str());
        else
            cluster_node.append_attribute("side").set_value("server");
        if (cluster.conformance.has_value())
            SerializeConformance(cluster.conformance.value(), cluster_node);
    }
}

} // namespace matter
