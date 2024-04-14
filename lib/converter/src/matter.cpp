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

int parseEvent(const pugi::xml_node& eventNode, eventType& event)
{
    return 0;
}

int parseCommand(const pugi::xml_node& commandNode, commandType& command)
{
    return 0;
}

int parseAttribute(const pugi::xml_node& attribute_node, attributeType& attribute)
{
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
            for (const auto& attribute_node : cluster_node.children("attribute")) {
                attributeType attribute;
                parseAttribute(attribute_node, attribute);
                cluster.attributes.push_back(attribute);
            }

            // Iterate through all commands and parse them individually
            for (const auto& command_node : cluster_node.children("command")) {
                commandType command;
                parseCommand(command_node, command);
                cluster.commands.push_back(command);
            }

            // Iterate through all events and parse them individually
            for (const auto& event_node : cluster_node.children("event")) {
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
