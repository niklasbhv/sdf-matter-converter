//
// Created by niklas on 05.03.24.
//

#include "matter.h"
#include "sdf.h"
#include <list>
#include <pugixml.hpp>

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

//TODO: Currently a workaround as enum is a occupied identifier
int parseEnum(pugi::xml_node& enum_type_node, enumType& enm)
{
    enm.name = enum_type_node.attribute("name").value();
    enm.type = enum_type_node.attribute("type").value();
    enm.cluster = enum_type_node.child("cluster").attribute("code").value();
    for (pugi::xml_node item_node : enum_type_node.children("item"))
    {
        enm.items.insert({item_node.attribute("name").value(), item_node.attribute("value").value()});
    }
    return 0;
}

int parseEvent(const pugi::xml_node& eventNode, eventType event)
{
    //event.description
    //event.access
    //event.field
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
    attribute.side = attribute_node.attribute("side").value();
    //attribute.description = attribute_node.child("description").value();
    //attribute.access =
    //attribute.code = attribute_node.attribute("code").as_int();
    attribute.define = attribute_node.attribute("define").value();
    attribute.type = attribute_node.attribute("type").value();
    //attribute.deflt = attribute_node.attribute("default").value();
    attribute.reportable = attribute_node.attribute("reportable").as_bool();
    attribute.writable = attribute_node.attribute("writable").as_bool();
    attribute.optional = attribute_node.attribute("optional").as_bool();
    attribute.isNullable = attribute_node.attribute("isNullable").as_bool();
    attribute.min = attribute_node.attribute("min").as_int();
    attribute.max = attribute_node.attribute("max").as_int();
    attribute.length = attribute_node.attribute("length").as_int();
    return 0;
}

int parseClusters(const pugi::xml_document& cluster_xml, std::list<clusterType>& clusterList)
{
    //! Iterate through all enum children
    std::list<enumType> enums;
    for (pugi::xml_node enum_node: cluster_xml.child("configurator").children("enum")){
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
        cluster.name = cluster_node.child("name").value();
        cluster.domain = cluster_node.child("domain").value();
        cluster.description = cluster_node.child("description").value();
        cluster.code = cluster_node.child("code").value();
        cluster.define = cluster_node.child("define").value();
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

int parseDevices(const pugi::xml_document& device_xml, std::list<deviceType>& deviceList)
{
    for (pugi::xml_node device_node: device_xml.child("configurator").children("deviceType")) {
        deviceType device;
        device.name = device_node.child("name").value();
        device.domain = device_node.child("domain").value();
        device.typeName = device_node.child("typeName").value();
        device.profileId = device_node.child("profileId").value();
        device.deviceId = device_node.child("deviceId").value();

        //! Iterate through all clusters children
        for (pugi::xml_node cluster_node: device_node.children("clusters")) {
            //TODO: It might be useful to match these to the definitions inside the cluster xml to minimize computation
            //TODO: On the other hand it could be overhead as we have to iterate through this list
            clusterType cluster;
            //TODO: Which of these do we need to keep? Consider round tripping
            cluster.name = cluster_node.attribute("cluster").value();
            cluster.client = cluster_node.attribute("client").value();
            cluster.server = cluster_node.attribute("server").value();
            //cluster_node.attribute("clientLocked").value();
            //cluster_node.attribute("serverLocked").value();
            device.clusters.push_back(cluster);
        }
        deviceList.push_back(device);
    }
    return 0;
}