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

namespace matter {

OtherQuality ParseOtherQuality(const pugi::xml_node& other_quality_node) {
    OtherQuality other_quality;
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

    if (!other_quality_node.attribute("diagnostics").empty()) //Check this
        other_quality.diagnostics = other_quality_node.attribute("diagnostics").as_bool();

    if (!other_quality_node.attribute("largeMessage").empty()) //Check this
        other_quality.large_message = other_quality_node.attribute("largeMessage").as_bool();

    if (!other_quality_node.attribute("quieterReporting").empty()) //Check this
        other_quality.quieter_reporting = other_quality_node.attribute("quieterReporting").as_bool();

    return other_quality;
}

Constraint ParseConstraint(const pugi::xml_node& constraint_node) {
    Constraint constraint;
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
    } else if (constraint.type == "between") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "min") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "max") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all

    // Octet string constraints
    // lengthAllowed
    else if (constraint.type == "lengthBetween") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "minLength") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "maxLength") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all

    // List constraints
    // countAllowed
    else if (constraint.type == "countBetween") {
        constraint.from = constraint_node.attribute("from").as_int();
        constraint.to = constraint_node.attribute("to").as_int();
    } else if (constraint.type == "minCount") {
        constraint.min = constraint_node.attribute("value").as_int();
    } else if (constraint.type == "maxCount") {
        constraint.max = constraint_node.attribute("value").as_int();
    }
    // all
    // Entry constraint -> Child named "entry"
    // Character string constraints

    return constraint;
}

int parse_logical_term(const pugi::xml_node& logical_node, std::string& condition) {
    std::string node_name = logical_node.name();
    if (node_name == "orTerm") {
        for (auto or_child: logical_node.child("orTerm").children()) {
            parse_logical_term(or_child, condition);
            condition.append("||");
        }
    } else if (node_name == "andTerm") {
        for (auto and_child: logical_node.child("andTerm").children()) {
            parse_logical_term(and_child, condition);
            condition.append("&&");
        }
    } else if (node_name == "xorTerm") {
        for (auto xor_child: logical_node.child("xorTerm").children()) {
            parse_logical_term(xor_child, condition);
            condition.append("^^");
        }
    } else if (node_name == "notTerm") {
        for (auto not_child: logical_node.child("notTerm").children()) {
            condition.append("!");
            parse_logical_term(not_child, condition);
        }
    }
    // Parentheses
    else if (node_name == "feature") {
        condition.append(logical_node.child("feature").attribute("name").value());
    } else if (node_name == "condition") {
        condition.append(logical_node.child("condition").attribute("name").value());
    } else if (node_name == "attribute") {
        condition.append(logical_node.child("attribute").attribute("name").value());
    }

    return 0;
}

Conformance ParseConformance(const pugi::xml_node& conformance_node) {
    Conformance conformance;
    // Mandatory conform
    if (!conformance_node.child("mandatoryConform").empty()) {
        conformance.mandatory = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("mandatoryConform").children().empty())
            parse_logical_term(conformance_node.child("mandatoryConform").first_child(), conformance.condition);
    }
    // Optional conform
    else if (!conformance_node.child("optionalConform").empty()) {
        conformance.optional = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("optionalConform").children().empty())
            parse_logical_term(conformance_node.child("optionalConform").first_child(), conformance.condition);
    }

    // Provisional conform
    else if (!conformance_node.child("provisionalConform").empty()) {
        conformance.provisional = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("provisionalConform").children().empty())
            parse_logical_term(conformance_node.child("provisionalConform").first_child(), conformance.condition);
    }
    // Deprecated conform
    else if (!conformance_node.child("deprecatedConform").empty()) {
        conformance.deprecated = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("deprecatedConform").children().empty())
            parse_logical_term(conformance_node.child("deprecatedConform").first_child(), conformance.condition);
    }
    // Disallowed conform
    else if (!conformance_node.child("disallowConform").empty()) {
        conformance.disallowed = true;
        // If the conformance has a child it is bound to a condition
        if (!conformance_node.child("disallowConform").children().empty())
            parse_logical_term(conformance_node.child("disallowConform").first_child(), conformance.condition);
    }
    // Otherwise conform
    else if (!conformance_node.child("otherwiseConform").empty()) {
        // Iterate through all child nodes
        for (auto otherwise_child: conformance_node.child("otherwiseConform").children()) {
            // Recursively process the child nodes
            // TODO: Currently the recursive processing with optional is pretty strange
            //std::optional<Conformance> otherwiseConformance;
            //ParseConformance(conformance_node.child("otherwiseConform"), otherwiseConformance);
            //conformance.otherwise.push_back(otherwiseConformance.value());
        }
    }

    // In case no conformance is defined
    else {
        //optional_conformance = std::nullopt;
    }

    return conformance;
}

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

CommonQuality ParseCommonQuality(const pugi::xml_node& common_quality_node) {
    CommonQuality common_quality;
    return common_quality;
}

Item ParseItem(const pugi::xml_node& enum_node) {
    Item item;
    // Iterate through all enum items and parse them individually
    for (const auto &enum_item_node: enum_node.children()) {
        //Item item;
        item.value = enum_item_node.attribute("value").as_int();
        item.name = enum_item_node.attribute("name").value();
        item.summary = enum_item_node.attribute("summary").value();

        item.conformance = ParseConformance(enum_item_node);

        //items.push_back(item);
    }

    return item;
}

Bitfield ParseBitfield(const pugi::xml_node& bitmap_node) {
    Bitfield bitfield;
    // Iterate through all bitfields and parse them individually
    for (const auto &bitfield_node: bitmap_node.children()) {
        //Bitfield bitfield;
        bitfield.bit = bitfield_node.attribute("bit").as_int();
        bitfield.name = bitfield_node.attribute("name").value();
        bitfield.summary = bitfield_node.attribute("summary").value();

        bitfield.conformance = ParseConformance(bitfield_node.child("conformance"));

        //bitfields.push_back(bitfield);
    }

    return bitfield;
}

DataField ParseDataField(const pugi::xml_node& data_fields_node) {
    DataField data_field;
    // Iterate through all struct fields and parse them individually
    for (const auto &data_field_node: data_fields_node.children()) {
        //DataField data_field;
        data_field.id = data_field_node.attribute("id").as_int();
        data_field.name = data_field_node.attribute("name").value();

        if (!data_field_node.child("access").empty())
            data_field.access = ParseAccess(data_field_node.child("access"));

        data_field.conformance = ParseConformance(data_field_node);

        if (!data_field_node.attribute("summary").empty())
            data_field.summary = data_field_node.attribute("summary").value();

        if (!data_field_node.attribute("type").empty())
            data_field.type = data_field_node.attribute("type").value();

        if (!data_field_node.child("constraint").empty())
            data_field.constraint = ParseConstraint(data_field_node.child("constraint"));

        if (!data_field_node.child("quality").empty())
            data_field.quality = ParseOtherQuality(data_field_node);

        //default
        //data_fields.push_back(data_field);
    }

    return data_field;
}

FeatureMap ParseFeatureMap(const pugi::xml_node& feature_map_node) {
    FeatureMap feature_map;
    // Iterate through all features and parse them individually
    for (const auto &feature: feature_map_node.children()) {
        //FeatureMap featureMap;
        feature_map.bit = feature.attribute("bit").as_int();
        feature_map.conformance = ParseConformance(feature);
        feature_map.code = feature.attribute("code").value();
        feature_map.name = feature.attribute("name").value();
        feature_map.summary = feature.attribute("summary").value();
        //featureMapList.push_back(featureMap);
    }

    return feature_map;
}

Event ParseEvent(const pugi::xml_node& event_node) {
    Event event;
    event.id = event_node.attribute("id").as_int();
    event.name = event_node.attribute("name").value();

    event.conformance = ParseConformance(event_node);

    if (!event_node.child("access").empty())
        event.access = ParseAccess(event_node.child("access"));

    event.summary = event_node.attribute("summary").value();
    event.priority = event_node.attribute("priority").value();
    auto quality_node = event_node.child("quality");
    if (!quality_node.empty())
        event.quality = ParseOtherQuality(quality_node);

    //event.data = ParseDataField(event_node);

    return event;
}

Command ParseCommand(const pugi::xml_node& command_node) {
    Command command;
    command.id = command_node.attribute("id").as_int();
    command.name = command_node.attribute("name").value();

    command.conformance = ParseConformance(command_node);

    if (!command_node.child("access").empty())
        command.access = ParseAccess(command_node.child("access"));

    command.summary = command_node.attribute("summary").value();
    // default
    command.direction = command_node.attribute("direction").value();
    command.response = command_node.attribute("response").value();

    return command;
}

Attribute ParseAttribute(const pugi::xml_node& attribute_node) {
    Attribute attribute;
    attribute.id = attribute_node.attribute("id").as_int();
    attribute.name = attribute_node.attribute("name").value();

    attribute.conformance = ParseConformance(attribute_node);

    if (!attribute_node.child("access").empty())
        attribute.access = ParseAccess(attribute_node.child("access"));

    attribute.summary = attribute_node.attribute("summary").value();
    attribute.type = attribute_node.attribute("type").value();
    auto quality_node = attribute_node.child("quality");
    if (!quality_node.empty())
        attribute.quality = ParseOtherQuality(quality_node);

    attribute.default_ = quality_node.attribute("default").value();

    return attribute;

}

/*
* Function used to parse globally defined custom data types.
*/
int parse_data_types(const pugi::xml_node& data_type_xml, Cluster& cluster) {
    // Parse all data types based on enums.

    for (const auto& enum_node: data_type_xml.children("enum")) {
        std::list<Item> enum_items;
        for (const auto& item_node : enum_node.children("item")) {
            Item item = ParseItem(enum_node);
            enum_items.push_back(item);
        }
        cluster.enums[enum_node.attribute("name").value()] = enum_items;
    }

    // Parse all data types based on bitmaps.

    for (const auto& bitmap_node: data_type_xml.children("bitmap")) {
        std::list<Bitfield> bitfields;
        for (const auto& bitfield_node : bitmap_node.children("bitfields")) {
            Bitfield bitfield = ParseBitfield(bitmap_node);
            bitfields.push_back(bitfield);
        }
        cluster.bitmaps[bitmap_node.attribute("name").value()] = bitfields;
    }

    return 0;
}

/*
* Function used to parse classification information.
*/
ClusterClassification ParseClusterClassification(const pugi::xml_node& classification_xml) {
    ClusterClassification cluster_classification;
    if (!classification_xml.attribute("hierarchy").empty())
        cluster_classification.hierarchy = classification_xml.attribute("hierarchy").value();

    if (!classification_xml.attribute("role").empty())
        cluster_classification.role = classification_xml.attribute("role").value();

    if (!classification_xml.attribute("picsCode").empty())
        cluster_classification.picsCode = classification_xml.attribute("picsCode").value();

    if (!classification_xml.attribute("scope").empty())
        cluster_classification.scope = classification_xml.attribute("scope").value();

    if (!classification_xml.attribute("baseCluster").empty())
        cluster_classification.base_cluster = classification_xml.attribute("baseCluster").value();

    if (!classification_xml.attribute("primaryTransaction").empty())
        cluster_classification.primary_transaction = classification_xml.attribute("primaryTransaction").value();

    return cluster_classification;
}

/*
* Function used to parse clusters.
*/
Cluster ParseCluster(const pugi::xml_node& cluster_xml) {
    Cluster cluster;
    //cluster.id
    cluster.name = cluster_xml.attribute("name").value();
    cluster.summary = cluster_xml.attribute("summary").value();
    cluster.revision = cluster_xml.attribute("revision").as_int();

    // Iterate through all revisions and parse them individually
    for (const auto &revision_node: cluster_xml.child("revisionHistory").children()) {
        cluster.revision_history.insert(
                {revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
    }

    // Parse the classification section
    if (!cluster_xml.child("classification").empty())
        cluster.classification = ParseClusterClassification(cluster_xml.child("classification"));

    // Parse the globally defined custom data types
    if (!cluster_xml.child("dataTypes").empty())
        parse_data_types(cluster_xml.child("dataTypes"), cluster);

    // Iterate through all attributes and parse them individually
    for (const auto &attribute_node: cluster_xml.child("attributes").children()) {
        cluster.attributes.push_back(ParseAttribute(attribute_node));
    }

    // Iterate through all commands and parse them individually
    for (const auto &command_node: cluster_xml.child("commands").children()) {
        // Split the commands into client and server commands
        if (command_node.attribute("direction").value() == "commandToServer") {
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

/*
* Function used to parse devices.
*/
Device ParseDevice(const pugi::xml_node& device_xml, bool client) {
    Device device;
    device.id = device_xml.attribute("id").as_int();
    device.name = device_xml.attribute("name").value();
    device.revision = device_xml.attribute("revision").as_int();
    std::cout << device.name << std::endl;

    // Iterate through all revisions and parse them individually
    for (const auto &revision_node: device_xml.child("revisionHistory").children()) {
        device.revision_history.insert(
                {revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
    }

    if (!device_xml.child("classification").empty())
        device.classification =  ParseDeviceClassification(device_xml.child("classification"));

    // Parse all data types and add them to a map
    auto data_type_node = device_xml.child("dataTypes");

    // Iterate through all clusters needed by the device and parse them individually
    for (const auto &cluster_node: device_xml.child("clusters").children("cluster")) {
        if (client) {
            if (cluster_node.attribute("side").value() == "client")
                device.clusters.push_back(ParseCluster(cluster_node));
        } else {
            if (cluster_node.attribute("side").value() == "server")
                device.clusters.push_back(ParseCluster(cluster_node));
        }
    }

    return device;
}

void SerializeOtherQuality(const OtherQuality& other_quality, pugi::xml_node& parent_node) {
    pugi::xml_node quality_node = parent_node.append_child("quality");
    if (other_quality.nullable.has_value())
        quality_node.append_attribute("nullable").set_value(other_quality.nullable.value());

    if (other_quality.non_volatile.has_value()) {
        quality_node.append_attribute("persistence").set_value("nonVolatile");
    } else if (other_quality.fixed.has_value()) {
        quality_node.append_attribute("persistence").set_value("fixed");
    } else {
        quality_node.append_attribute("persistence").set_value("volatile");
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

/*
* Function used to create a xml attribute for default types
*/
pugi::xml_attribute SerializeDefaultType(const DefaultType &value, const char *attribute_name) {
    pugi::xml_attribute attribute;
    attribute.set_name(attribute_name);
    if (std::holds_alternative<double>(value))
        attribute.set_value(std::get<double>(value));

    else if (std::holds_alternative<int64_t>(value))
        attribute.set_value(std::get<int64_t>(value));

    else if (std::holds_alternative<uint64_t>(value))
        attribute.set_value(std::get<uint64_t>(value));

    else if (std::holds_alternative<std::string>(value))
        attribute.set_value(std::get<std::string>(value).c_str());

    return attribute;
}

/*
* Function used to set the attribute value for the numeric Type
*/
pugi::xml_attribute SerializeNumericType(const NumericType& value, const char* attribute_name) {
    pugi::xml_attribute attribute;
    attribute.set_name(attribute_name);
    if (std::holds_alternative<double>(value))
        attribute.set_value(std::get<double>(value));

    else if (std::holds_alternative<int64_t>(value))
        attribute.set_value(std::get<int64_t>(value));

    else if (std::holds_alternative<uint64_t>(value))
        attribute.set_value(std::get<uint64_t>(value));

    return attribute;
}

void SerializeConstraint(const Constraint& constraint, pugi::xml_node& parent_node) {
    auto constraint_node = parent_node.append_child("constraint");
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

        constraint_node.append_copy(SerializeDefaultType(constraint.value.value(), "value"));
    } else if (constraint.type == "between") {
        constraint_node.append_attribute("type").set_value("between");

        constraint_node.append_copy(SerializeNumericType(constraint.from.value(), "from"));

        constraint_node.append_copy(SerializeNumericType(constraint.to.value(), "to"));
    } else if (constraint.type == "min") {
        constraint_node.append_attribute("type").set_value("min");

        constraint_node.append_copy(SerializeNumericType(constraint.min.value(), "value"));
    } else if (constraint.type == "max") {
        constraint_node.append_attribute("type").set_value("max");

        constraint_node.append_copy(SerializeNumericType(constraint.max.value(), "value"));
    }
    // all

    // Octet string constraints
    // lengthAllowed
    else if (constraint.type == "lengthBetween") {
        constraint_node.append_attribute("type").set_value("lengthBetween");

        constraint_node.append_copy(SerializeNumericType(constraint.from.value(), "from"));

        constraint_node.append_copy(SerializeNumericType(constraint.to.value(), "to"));
    } else if (constraint.type == "minLength") {
        constraint_node.append_attribute("type").set_value("minLength");

        constraint_node.append_copy(SerializeNumericType(constraint.min.value(), "value"));
    } else if (constraint.type == "maxLength") {
        constraint_node.append_attribute("type").set_value("maxLength");

        constraint_node.append_copy(SerializeNumericType(constraint.max.value(), "value"));
    }
    // all

    // List constraints
    // countAllowed
    else if (constraint.type == "countBetween") {
        constraint_node.append_attribute("type").set_value("countBetween");

        constraint_node.append_copy(SerializeNumericType(constraint.from.value(), "from"));

        constraint_node.append_copy(SerializeNumericType(constraint.to.value(), "to"));;
    } else if (constraint.type == "minCount") {
        constraint_node.append_attribute("type").set_value("minCount");

        constraint_node.append_copy(SerializeNumericType(constraint.min.value(), "value"));
    } else if (constraint.type == "maxCount") {
        constraint_node.append_attribute("type").set_value("maxCount");

        constraint_node.append_copy(SerializeNumericType(constraint.max.value(), "value"));
    }
    // all
    // Entry constraint -> Child named "entry"
    // Character string constraints
}

void SerializeConformance(const Conformance& conformance, pugi::xml_node& parent_node) {
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value())
            parent_node.append_child("mandatoryConformance");
    } else if (conformance.optional.has_value()) {
        if (conformance.optional.value())
            parent_node.append_child("optionalConformance");
    } else if (conformance.provisional.has_value()) {
        if (conformance.provisional.value())
            parent_node.append_child("provisionalConformance");
    } else if (conformance.deprecated.has_value()) {
        if (conformance.deprecated.value())
            parent_node.append_child("deprecatedConformance");
    } else if (conformance.disallowed.has_value()) {
        if (conformance.disallowed.value())
            parent_node.append_child("disallowedConformance");
    }
    // expression
}

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

void SerializeEvent(const Event& event, pugi::xml_node& events_node) {
    auto event_node = events_node.append_child("event");
}

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

    // default
}

void SerializeCommand(const Command& command, pugi::xml_node& commands_node) {
    auto command_node = commands_node.append_child("command");
    command_node.append_attribute("id").set_value(DecToHexa(command.id).c_str());
    command_node.append_attribute("name").set_value(command.name.c_str());

    if (command.conformance.has_value())
       SerializeConformance(command.conformance.value(), command_node);

    if (command.access.has_value())
        SerializeAccess(command.access.value(), command_node);

    if (!command.summary.empty())
        command_node.append_attribute("summary").set_value(command.summary.c_str());

    //if (!command.default_.empty())
    //    command_node.append_attribute("default").set_value(command.default_.c_str());

    command_node.append_attribute("direction").set_value(command.direction.c_str());
    command_node.append_attribute("response").set_value(command.response.c_str());

    // Serialize the command fields
    for (const auto &field: command.command_fields) {
        SerializeDataField(field, command_node);
    }
}

void SerializeAttribute(const Attribute& attribute, pugi::xml_node& attributes_node) {
    auto attribute_node = attributes_node.append_child("attribute");

    attribute_node.append_attribute("id").set_value(DecToHexa(attribute.id).c_str());
    attribute_node.append_attribute("name").set_value(attribute.name.c_str());

    if (attribute.conformance.has_value())
        SerializeConformance(attribute.conformance.value(), attribute_node);

    if (attribute.access.has_value())
        SerializeAccess(attribute.access.value(), attribute_node);

    if (!attribute.summary.empty())
        attribute_node.append_attribute("summary").set_value(attribute.summary.c_str());

    if (attribute.quality.has_value())
        SerializeOtherQuality(attribute.quality.value(), attribute_node);

    attribute_node.attribute("type").set_value(attribute.type.c_str());
    //attribute_node.attribute("default").set_value(attribute.default_.c_str());
}

void SerializeItem(const Item& item, pugi::xml_node& enum_node) {
    auto item_node = enum_node.append_child("item");
    item_node.append_attribute("value").set_value(item.value);
    item_node.append_attribute("name").set_value(item.name.c_str());
    item_node.append_attribute("summary").set_value(item.summary.c_str());
    if (item.conformance.has_value())
        SerializeConformance(item.conformance.value(), item_node);
}

void SerializeBitfield(const Bitfield& bitfield, pugi::xml_node& bitmap_node) {
    auto bitfield_node = bitmap_node.append_child("bitfield");
    bitfield_node.append_attribute("name").set_value(bitfield.name.c_str());
    bitfield_node.append_attribute("bit").set_value(bitfield.bit);
    bitfield_node.append_attribute("summary").set_value(bitfield.summary.c_str());
    if (bitfield.conformance.has_value())
        SerializeConformance(bitfield.conformance.value(), bitfield_node);
}

void SerializeDataTypes(const Cluster& cluster, pugi::xml_node& cluster_xml) {
    auto data_type_node = cluster_xml.append_child("dataTypes");

    // number
    // struct

    for (const auto &current_enum: cluster.enums) {
        pugi::xml_node enum_node = data_type_node.append_child("enum");
        enum_node.append_attribute("name").set_value(current_enum.first.c_str());
        for (const auto &enum_item: current_enum.second) {
            SerializeItem(enum_item, enum_node);
        }
    }

    for (const auto &bitmap: cluster.bitmaps) {
        pugi::xml_node bitmap_node = data_type_node.append_child("bitmap");
        bitmap_node.append_attribute("name").set_value(bitmap.first.c_str());
        for (const auto &bitfield: bitmap.second) {
            pugi::xml_node bitfield_node = bitmap_node.append_child("bitfield");
            SerializeBitfield(bitfield, bitmap_node);
        }
    }
}

void SerializeClusterClassification(const ClusterClassification& cluster_classification, pugi::xml_node cluster_node) {
    auto classification_node = cluster_node.append_child("classification");
    if (!cluster_classification.hierarchy.empty())
        classification_node.append_attribute("hierarchy").set_value(cluster_classification.hierarchy.c_str());

    if (!cluster_classification.role.empty())
        classification_node.append_attribute("role").set_value(cluster_classification.role.c_str());

    if (!cluster_classification.picsCode.empty())
        classification_node.append_attribute("picsCode").set_value(cluster_classification.picsCode.c_str());

    if (!cluster_classification.scope.empty())
        classification_node.append_attribute("scope").set_value(cluster_classification.scope.c_str());

    if (!cluster_classification.base_cluster.empty())
        classification_node.append_attribute("base_cluster").set_value(cluster_classification.base_cluster.c_str());

    if (!cluster_classification.primary_transaction.empty())
        classification_node.append_attribute("primary_transaction").set_value(
                cluster_classification.primary_transaction.c_str());
}

pugi::xml_document SerializeCluster(const Cluster &cluster) {
    pugi::xml_document cluster_xml;
    // Create the cluster node
    auto cluster_node = cluster_xml.append_child("cluster");

    cluster_node.append_attribute("id").set_value(DecToHexa(cluster.id).c_str());
    cluster_node.append_attribute("name").set_value(cluster.name.c_str());

    if (cluster.conformance.has_value())
        SerializeConformance(cluster.conformance.value(), cluster_node);

    if (cluster.access.has_value())
        SerializeAccess(cluster.access.value(), cluster_node);

    if (!cluster.summary.empty())
        cluster_node.append_attribute("summary").set_value(cluster.summary.c_str());

    // Serialize the classification information
    if (cluster.classification.has_value())
        SerializeClusterClassification(cluster.classification.value(), cluster_node);

    // Iterate through all revisions and serialize them individually
    auto revision_history_node = cluster_node.append_child("revisionHistory");
    for (const auto &revision: cluster.revision_history) {
        auto revision_node = revision_history_node.append_child("revision");
        revision_node.append_attribute("revision").set_value(revision.first);
        revision_node.append_attribute("summary").set_value(revision.second.c_str());
    }

    // Serialize the custom data types
    SerializeDataTypes(cluster, cluster_node);
    // Iterate through all attributes and serialize them individually
    auto attributes_node = cluster_node.append_child("attributes");
    for (const auto &attribute: cluster.attributes) {
        SerializeAttribute(attribute, attributes_node);
    }

    // Iterate through all client and server commands and serialize them individually
    auto commands_node = cluster_node.append_child("commands");
    for (const auto& client_command : cluster.client_commands) {
        SerializeCommand(client_command, commands_node);
    }
    for (const auto& server_command : cluster.server_commands) {
        SerializeCommand(server_command.second, commands_node);
    }

    // Iterate through all events and serialize them individually
    auto events_node = cluster_node.append_child("events");
    for (const auto &event: cluster.events) {
        SerializeEvent(event, events_node);
    }

    return cluster_xml;
}

void SerializeDeviceClassification(const DeviceClassification& device_classification, pugi::xml_node& device_node) {
    auto classification_node = device_node.append_child("classification");
    if (!device_classification.superset.empty())
        classification_node.append_attribute("superset").set_value(device_classification.superset.c_str());

    if (!device_classification.class_.empty())
        classification_node.append_attribute("class").set_value(device_classification.class_.c_str());

    if (!device_classification.scope.empty())
        classification_node.append_attribute("scope").set_value(device_classification.scope.c_str());
}

pugi::xml_document SerializeDevice(const Device& device) {
    pugi::xml_document device_document_xml;
    pugi::xml_node device_xml = device_document_xml.document_element();
    auto device_node = device_xml.append_child("Device");

    device_node.append_attribute("id").set_value(DecToHexa(device.id).c_str());
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

    // Iterate through all clusters and serialize them individually
    auto clusters_node = device_node.append_child("clusters");
    for (const auto &cluster: device.clusters) {
        clusters_node.append_move(SerializeCluster(cluster).document_element());
    }

    return device_document_xml;
}

} // namespace matter
