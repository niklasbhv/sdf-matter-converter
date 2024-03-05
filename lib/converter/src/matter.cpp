//
// Created by niklas on 05.03.24.
//

#include "matter.h"
#include "sdf.h"
#include <list>
#include <pugixml.hpp>



//
// Functions responsible for the Matter -> SDF conversion
//

int mapBitmap(pugi::xml_node& bitmap_type_node, bitmapType& bitmap)
{
    bitmap.name = bitmap_type_node.attribute("name").value();
    bitmap.type = bitmap_type_node.attribute("type").value();
    bitmap.cluster = bitmap_type_node.child("cluster").attribute("code").value();
    for (pugi::xml_node field_node : bitmap_type_node.children("field"))
    {
        //TODO: Should this be a map?
        bitmap.fields.insert({field_node.attribute("name").value(), field_node.attribute("mask").value()});
    }
    return 0;
}

//TODO: Currently a workaround as enum is a occupied identifier
int mapEnum(pugi::xml_node& enum_type_node, enumType& enm)
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

int mapEvent()
{
    return 0;
}

int mapCommand()
{
    return 0;
}

int mapAttribute()
{
    return 0;
}

int mapCluster(const pugi::xml_document& cluster_xml)
{
    //TODO: We have to iterate through custom type definitions beforehand, below we only iterate trough the clusters
    //! Iterate through all enum children
    std::list<enumType> enums;
    for (pugi::xml_node enum_node: cluster_xml.child("configurator").children("enum")){
        enumType enm;
        mapEnum(enum_node, enm);
        enums.push_back(enm);
    }

    //! Iterate through all bitmap children
    std::list<bitmapType> bitmaps;
    for (pugi::xml_node bitmap_node: cluster_xml.child("configurator").children("bitmap")){
        bitmapType bitmap;
        mapBitmap(bitmap_node, bitmap);
        bitmaps.push_back(bitmap);
    }

    //TODO: Currently only a single Cluster is possible, should be multiple
    clusterType cluster;
    //! Iterate through all cluster children
    for (pugi::xml_node cluster_node: cluster_xml.child("configurator").children("cluster")){
        cluster.name = cluster_node.child("name").value();
        cluster.domain = cluster_node.child("domain").value();
        cluster.code = cluster_node.child("code").value();
        cluster.define = cluster_node.child("define").value();
        cluster.description  = cluster_node.child("description").value();
    }
    return 0;
}

int mapDevice(const pugi::xml_node& device_type_node, deviceType& device)
{
    device.name = device_type_node.child("name").value();
    device.domain = device_type_node.child("domain").value();
    device.typeName = device_type_node.child("typeName").value();
    device.profileId = device_type_node.child("profileId").value();
    device.deviceId = device_type_node.child("deviceId").value();
    //! Iterate through all clusters children
    for (pugi::xml_node cluster_node: device_type_node.children("clusters"))
    {
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
    return 0;
}