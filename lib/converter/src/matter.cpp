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

//TODO: Trying to interpret strings as types can fail and should be caught
//TODO: Many of these are optional, how does the code react to missing attributes?

int parseBitmap(pugi::xml_node& bitmap_type_node, bitmapType& bitmap)
{
    bitmap.name = bitmap_type_node.attribute("name").value();
    bitmap.type = bitmap_type_node.attribute("type").value();
    //TODO: This can be 0 or n-ary
    bitmap.cluster = bitmap_type_node.child("cluster").attribute("code").value();

    for (pugi::xml_node field_node : bitmap_type_node.children("field"))
    {
        bitmap.fields.push_back(fieldType{
                field_node.attribute("mask").value(),
                field_node.attribute("name").value(),
                field_node.attribute("introducedIn").value(),
                field_node.attribute("fieldId").as_int(),
        });
    }

    return 0;
}

int parseEnum(pugi::xml_node& enum_type_node, enumType& enum_)
{
    enum_.name = enum_type_node.attribute("name").value();
    enum_.type = enum_type_node.attribute("type").value();
    enum_.cluster = enum_type_node.child("cluster").attribute("code").value();

    for (pugi::xml_node item_node : enum_type_node.children("item"))
    {
        enum_.items.insert({item_node.attribute("name").value(), item_node.attribute("value").value()});
    }

    return 0;
}

int parseEvent(const pugi::xml_node& eventNode, eventType& event)
{
    event.description = eventNode.child("description").value();

    for (const pugi::xml_node& accessNode : eventNode.children("access")){
        accessType access;
        access.op = accessNode.attribute("op").value();
        access.role = accessNode.attribute("role").value();
        access.privilege = accessNode.attribute("privilege").value();
        access.modifier = accessNode.attribute("modifier").value();
        event.access.push_back(access);
    }

    for (const pugi::xml_node& fieldNode : eventNode.children("field")){
        eventFieldType eventField;
        eventField.id = fieldNode.attribute("id").value();
        eventField.name = fieldNode.attribute("name").value();
        eventField.type = fieldNode.attribute("type").value();
        eventField.array = fieldNode.attribute("array").as_bool();
        eventField.isNullable = fieldNode.attribute("isNullable").as_bool();
        event.field.push_back(eventField);
    }

    event.code = eventNode.attribute("code").value();
    event.name = eventNode.attribute("name").value();
    event.side = eventNode.attribute("side").value();
    event.priority = eventNode.attribute("priority").value();

    return 0;
}

int parseCommand(const pugi::xml_node& commandNode, commandType& command)
{
    //command.description
    //command.access

    for (pugi::xml_node arg_node : commandNode.children("arg")) {
        //TODO: Complete these
        argType arg;
        arg.name = arg_node.attribute("name").value();
        arg.type.name = arg_node.attribute("type").value();
        command.arg.push_back(arg);
    }

    //command.cli = commandNode.attribute("cli").value();
    command.cliFunctionName = commandNode.attribute("cliFunctionName").value();
    command.code = commandNode.attribute("code").value();
    command.disableDefaultResponse = commandNode.attribute("disableDefaultResponse").as_bool();
    command.functionName = commandNode.attribute("functionName").value();
    command.group = commandNode.attribute("group").value();
    command.introducedIn = commandNode.attribute("introducedIn").value();
    command.noDefaultImplementation = commandNode.attribute("noDefaultImplementation").as_bool();
    command.manufacturerCode = commandNode.attribute("manufacturerCode").value();
    command.name = commandNode.attribute("name").value();
    command.optional = commandNode.attribute("optional").as_bool();
    command.source = commandNode.attribute("source").value();
    command.restriction = commandNode.attribute("restriction").value();
    command.response = commandNode.attribute("response").value();

    return 0;
}

int parseAttribute(const pugi::xml_node& attribute_node, attributeType& attribute)
{
    attribute.name = attribute_node.child_value();
    if (attribute.name.empty())
        attribute.name = attribute_node.child("description").child_value();
    //attribute.description = attribute_node.child("description").child_value();

    for (const pugi::xml_node& accessNode : attribute_node.children("access")){
        accessType access;
        access.op = accessNode.attribute("op").value();
        access.role = accessNode.attribute("role").value();
        access.privilege = accessNode.attribute("privilege").value();
        access.modifier = accessNode.attribute("modifier").value();
    }

    attribute.code = attribute_node.attribute("code").value();
    attribute.default_ = attribute_node.attribute("default").value();
    attribute.define = attribute_node.attribute("define").value();
    attribute.introducedIn = attribute_node.attribute("introducedIn").value();
    if (!attribute_node.attribute("length").empty())
        attribute.length = attribute_node.attribute("length").as_int();
    attribute.manufacturerCode = attribute_node.attribute("manufacturerCode").value();
    if (!attribute_node.attribute("max").empty())
        attribute.max = attribute_node.attribute("max").as_int();
    if (!attribute_node.attribute("min").empty())
        attribute.min = attribute_node.attribute("min").as_int();
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    attribute.optional = attribute_node.attribute("optional").as_bool();
    attribute.side = attribute_node.attribute("side").value();
    attribute.type = attribute_node.attribute("type").value();
    attribute.readable = attribute_node.attribute("readable").as_bool();
    attribute.writable = attribute_node.attribute("writable").as_bool();
    attribute.reportable = attribute_node.attribute("reportable").as_bool();
    attribute.array = attribute_node.attribute("array").as_bool();
    attribute.isNullable = attribute_node.attribute("isNullable").as_bool();

    return 0;
}

int parseCluster(const pugi::xml_node& cluster_xml, clusterType& cluster)
{
    // TODO: Currently enums and bitmaps are parsed for every cluster, even if they're not needed
    // Iterate through all enum children
    std::list<enumType> enums;
    for (pugi::xml_node enum_node: cluster_xml.children("enum")){
        enumType enm;
        parseEnum(enum_node, enm);
        enums.push_back(enm);
    }

    // Iterate through all bitmap children
    std::list<bitmapType> bitmaps;
    for (pugi::xml_node bitmap_node: cluster_xml.children("bitmap")){
        bitmapType bitmap;
        parseBitmap(bitmap_node, bitmap);
        bitmaps.push_back(bitmap);
    }

    // Iterate through all clusters children
    for (auto cluster_node : cluster_xml.children("cluster")){
        // Search for the matching cluster definition
        if (cluster.name == cluster_node.child("name").child_value()){
            cluster.domain = cluster_node.child("domain").child_value();
            cluster.description = cluster_node.child("description").child_value();
            cluster.code = cluster_node.child("code").child_value();
            cluster.define = cluster_node.child("define").child_value();
            // cluster.generateCmdHandlers
            // cluster.tag
            // cluster.globalAttribute

            // Iterate through all attribute children
            for (pugi::xml_node attribute_node: cluster_node.children("attribute")) {
                attributeType attribute;
                parseAttribute(attribute_node, attribute);
                cluster.attributes.push_back(attribute);
            }

            // Iterate through all command children
            for (pugi::xml_node command_node: cluster_node.children("command")) {
                commandType command;
                parseCommand(command_node, command);
                cluster.commands.push_back(command);
            }

            // Iterate through all event children
            for (pugi::xml_node event_node: cluster_node.children("event")) {
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
    pugi::xml_node device_node = device_xml.child("deviceType");
    device.name = device_node.child("name").child_value();
    device.domain = device_node.child("domain").child_value();
    device.typeName = device_node.child("typeName").child_value();
    device.profileId = device_node.child("profileId").child_value();
    device.deviceId = device_node.child("deviceId").child_value();

    // Iterate through all clusters children
    // Each cluster gets parsed afterward, this way only clusters that are used for a device are parsed
    for (pugi::xml_node cluster_node: device_node.child("clusters").children()) {
        clusterType cluster;
        cluster.name = cluster_node.attribute("cluster").value();
        cluster.client = cluster_node.attribute("client").as_bool();
        cluster.server = cluster_node.attribute("server").as_bool();
        //cluster_node.attribute("clientLocked").value();
        //cluster_node.attribute("serverLocked").value();
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
    // TODO: Depending on which of these are necessary, check if fields are empty
    // domain
    pugi::xml_node cluster_node = cluster_xml.append_child("configurator").append_child("cluster");
    cluster_node.append_child("name").text().set(cluster.name.c_str());
    cluster_node.append_child("domain").text().set(cluster.domain.c_str());
    cluster_node.append_child("code").text().set(cluster.code.c_str());
    cluster_node.append_child("define").text().set(cluster.define.c_str());
    cluster_node.append_child("description").text().set(cluster.description.c_str());

    for (const auto& attribute : cluster.attributes) {
        serializeAttribute(attribute, cluster_node);
    }

    for (const auto& command : cluster.commands) {
        serializeCommand(command, cluster_node);
    }

    for (const auto& event : cluster.events) {
        serializeEvent(event, cluster_node);
    }

    return 0;
}

int serializeDevice(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml)
{
    // TODO: Depending on which of these are necessary, check if fields are empty
    pugi::xml_node device_node = device_xml.append_child("configurator").append_child("deviceType");
    device_node.append_child("name").text().set(device.name.c_str());
    device_node.append_child("domain").text().set(device.domain.c_str());
    device_node.append_child("typeName").text().set(device.typeName.c_str());
    device_node.append_child("profileId").text().set(device.profileId.c_str());
    device_node.append_child("deviceId").text().set(device.deviceId.c_str());

    pugi::xml_node clusters_node = device_node.append_child("clusters");
    for (const auto& cluster : device.clusters) {
        pugi::xml_node cluster_node = clusters_node.append_child("include");
        cluster_node.attribute("cluster").set_value(cluster.name.c_str());
        cluster_node.attribute("client").set_value(cluster.client);
        cluster_node.attribute("server").set_value(cluster.server);
        //cluster_node.attribute("clientLocked").set_value(cluster.clientLocked);
        //cluster_node.attribute("serverLocked").set_value(cluster.serverLocked);
        serializeCluster(cluster, cluster_xml);
    }

    return 0;
}
