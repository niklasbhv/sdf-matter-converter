#include "matter.h"
#include "sdf.h"
#include <list>
#include <pugixml.hpp>
#include <iostream>

//TODO: Trying to interpret strings as types can fail and should be caught
//TODO: Many of these are optional, how does the code react to missing attributes?

//
// Functions responsible for the Matter -> SDF conversion
//

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

int parseEvent(const pugi::xml_node& eventNode, eventType event)
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

int parseCommand(const pugi::xml_node& commandNode, commandType command)
{
    //command.description
    //command.access
    //command.arg
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
    attribute.name = attribute_node.value();
    attribute.description = attribute_node.child("description").value();
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
    attribute.length = attribute_node.attribute("length").as_int();
    attribute.manufacturerCode = attribute_node.attribute("manufacturerCode").value();
    attribute.max = attribute_node.attribute("max").as_int();
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

int parseClusters(const pugi::xml_node& cluster_xml, std::list<clusterType>& clusterList)
{
    //! Iterate through all enum children
    std::list<enumType> enums;
    for (pugi::xml_node enum_node: cluster_xml.children("enum")){
        enumType enm;
        parseEnum(enum_node, enm);
        enums.push_back(enm);
    }

    //! Iterate through all bitmap children
    std::list<bitmapType> bitmaps;
    for (pugi::xml_node bitmap_node: cluster_xml.child("configurator").children("bitmap")){
        bitmapType bitmap;
        parseBitmap(bitmap_node, bitmap);
        bitmaps.push_back(bitmap);
    }

    //! Iterate through all clusters children
    for (pugi::xml_node cluster_node: cluster_xml.child("configurator").children("cluster")) {
        clusterType cluster;
        cluster.name = cluster_node.child("name").child_value();
        cluster.domain = cluster_node.child("domain").child_value();
        cluster.description = cluster_node.child("description").child_value();
        cluster.code = cluster_node.child("code").child_value();
        cluster.define = cluster_node.child("define").child_value();
        // cluster.server
        // cluster.client
        // cluster.generateCmdHandlers
        // cluster.tag
        // cluster.globalAttribute

        //! Iterate through all attribute children
        std::list<attributeType> attributeList;
        for (pugi::xml_node attribute_node: cluster_node.children("attribute")) {
            attributeType attribute;
            parseAttribute(attribute_node, attribute);
            attributeList.push_back(attribute);
        }
        cluster.attributes = attributeList;

        //! Iterate through all command children
        std::list<commandType> commandList;
        for (pugi::xml_node command_node: cluster_node.children("command")) {
            commandType command;
            parseCommand(command_node, command);
            commandList.push_back(command);
        }
        cluster.commands = commandList;

        //! Iterate through all event children
        std::list<eventType> eventList;
        for (pugi::xml_node event_node: cluster_node.children("event")) {
            eventType event;
            parseEvent(event_node, event);
            eventList.push_back(event);
        }
        cluster.events = eventList;

        clusterList.push_back(cluster);
    }
    return 0;
}

int parseDevice(const pugi::xml_node& device_xml, deviceType& device)
{
        std::cout << device_xml.name() << std::endl;
        auto device_node = device_xml.child("deviceType");
        device.name = device_node.child("name").child_value();
        std::cout << device_node.name() << std::endl;
        device.domain = device_node.child("domain").child_value();
        std::cout << device.domain << std::endl;
        device.typeName = device_node.child("typeName").child_value();
        std::cout << device.typeName << std::endl;
        device.profileId = device_node.child("profileId").child_value();
        std::cout << device.profileId << std::endl;
        device.deviceId = device_node.child("deviceId").child_value();
        std::cout << device.deviceId << std::endl;

        //! Iterate through all clusters children
        for (pugi::xml_node cluster_node: device_node.children("clusters")) {
            //TODO: It might be useful to match these to the definitions inside the cluster xml to minimize computation
            //TODO: On the other hand it could be overhead as we have to iterate through this list
            clusterType cluster;
            //TODO: Which of these do we need to keep? Consider round tripping
            cluster.name = cluster_node.attribute("cluster").value();
            //cluster.client = cluster_node.attribute("client").value();
            //cluster.server = cluster_node.attribute("server").value();
            //cluster_node.attribute("clientLocked").value();
            //cluster_node.attribute("serverLocked").value();
            device.clusters.push_back(cluster);
        }
    return 0;
}
