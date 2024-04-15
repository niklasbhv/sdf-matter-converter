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
        conformance.supported = true;
    }
    // Optional conform
    else if (!conformance_node.child("optionalConform").empty()) {
        conformance.mandatory = false;
        conformance.supported = true;
    }
    // Provisional conform

    // Deprecated conform

    // Disallowed conform

    return 0;
}

int parseAccess(const pugi::xml_node& access_node, accessType& access)
{
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
    attribute.default_ = attribute_node.attribute("default").value();
    return 0;
}

int parseCluster(const pugi::xml_node& cluster_xml, clusterType& cluster)
{
    // Search for the matching cluster inside the cluster xml definitions
    for (const auto& cluster_node : cluster_xml.child("cluster")) {
        // Match the cluster definitions with their unique id
        if (cluster_node.attribute("id").as_int() == cluster.id) {
            cluster.revision = cluster_node.attribute("revision").as_int();

            // Iterate through all revisions and parse them individually
            for (const auto& revision_node : cluster_node.child("revisionHistory").children()) {
                cluster.revision_history.insert({revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
            }
            // classification
            // Iterate through all attributes and parse them individually
            for (const auto& attribute_node : cluster_node.child("attributes").children()) {
                attributeType attribute;
                parseAttribute(attribute_node, attribute);
                cluster.attributes.push_back(attribute);
            }

            // Iterate through all commands and parse them individually
            for (const auto& command_node : cluster_node.child("commands").children()) {
                commandType command;
                parseCommand(command_node, command);
                cluster.commands.push_back(command);
            }

            // Iterate through all events and parse them individually
            for (const auto& event_node : cluster_node.child("events").children()) {
                eventType event;
                parseEvent(event_node, event);
                cluster.events.push_back(event);
            }
        }
    }

    return 0;
}

int parseDevice(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device)
{
    auto device_node = device_xml.child("deviceType");

    device.id = device_node.attribute("id").as_int();
    device.name = device_node.attribute("name").value();
    device.revision = device_node.attribute("revision").as_int();

    // Iterate through all revisions and parse them individually
    for (const auto& revision_node : device_xml.child("revisionHistory").children()) {
        device.revision_history.insert({revision_node.attribute("revision").as_int(), revision_node.attribute("summary").value()});
    }

    // TODO:  Insert classification
    // Iterate through all clusters needed by the device and parse them
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
