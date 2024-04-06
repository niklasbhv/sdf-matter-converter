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

#include <iostream>
#include "mapping.h"
#include "matter.h"
#include "sdf.h"

int resolve_mappings(clusterType& cluster)
{
    return 0;
}

int save_to_mapping(const char* name, const std::string& value)
{
    //deviceTree.append_attribute(name) = value.c_str();
    return 0;
}

template <typename T>

int get_from_mapping(const char* name, T& value)
{
    return 0;
}

int map_sdf_event(sdfEventType& sdfEvent, eventType& event)
{
    event.name = sdfEvent.label;
    event.description = sdfEvent.description;
    // access
    // field
    // code
    // side
    // priority
    return 0;
}

int map_sdf_action(sdfActionType& sdfAction, commandType& command)
{
    command.name = sdfAction.label;
    command.description = sdfAction.description;
    //TODO: This needs further mapping as these need to be split up
    return 0;
}

int map_sdf_property(sdfPropertyType& sdfProperty, attributeType& attribute)
{
    attribute.name = sdfProperty.label;
    attribute.description = sdfProperty.description;
    // access
    // code
    attribute.default_ = sdfProperty.default_;
    // define
    // introducedIn
    // length
    // manufacturerCode
    //TODO: Does this match?
    attribute.max = sdfProperty.maximum;
    attribute.min = sdfProperty.minimum;
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    // optional
    // side
    attribute.type = sdfProperty.type; //TODO: Definitely needs mapping
    // reportable
    // array
    attribute.isNullable = sdfProperty.nullable;
    attribute.readable = sdfProperty.readable;
    attribute.writable = sdfProperty.writable;
    return 0;
}

int map_sdf_object(sdfObjectType& sdfObject, clusterType& cluster)
{
    //! Common qualities
    cluster.name = sdfObject.label;
    cluster.description = sdfObject.description;
    save_to_mapping("domain", cluster.domain);
    save_to_mapping("code", cluster.code);
    // define
    // server
    // client
    // generateCmdHandlers
    // tag
    // globalAttribute
    save_to_mapping("introducedIn", cluster.introducedIn);
    save_to_mapping("manufacturerCode", cluster.manufacturerCode);
    // singleton

    //! Iterate through sdfProperties
    for (auto sdfProperty : sdfObject.sdfProperty){
        attributeType attribute;
        map_sdf_property(sdfProperty.second, attribute);
        cluster.attributes.push_back(attribute);
    }

    //! Iterate through sdfActions
    for (auto sdfAction : sdfObject.sdfAction){
        commandType command;
        map_sdf_action(sdfAction.second, command);
        cluster.commands.push_back(command);
    }

    //! Iterate through sdfEvents
    for (auto sdfEvent : sdfObject.sdfEvent){
        eventType event;
        map_sdf_event(sdfEvent.second, event);
        cluster.events.push_back(event);
    }
    return 0;
}

int map_sdf_thing(sdfThingType& sdfThing, deviceType& device)
{
    //! Common qualities
    device.name = sdfThing.label;
    // domain
    // typeName
    save_to_mapping("profileId", device.profileId);
    save_to_mapping("deviceId", device.deviceId);
    // channels
    for (auto sdfObject : sdfThing.sdfObject){
        clusterType cluster;
        map_sdf_object(sdfObject.second, cluster);
        device.clusters.push_back(cluster);
    }
    //TODO: How do we handle Properties, Actions and Events of a sdfThing?
    return 0;
}

int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType, deviceType& device)
{
    //! Make the SDF mapping global
    //MappingList.merge(sdfMappingType.map);

    std::list<clusterType> clusterList;
    if (sdfModel.sdfThing.has_value()){
        map_sdf_thing(sdfModel.sdfThing.value(), device);
    } else if (sdfModel.sdfObject.has_value()){
        // TODO: If no sdfThings are present, a new device with a single cluster has to be created
    }
    // TODO: How do we handle SDF Mappings?

    return 0;
}

int map_matter_type(std::string& matter_type, std::string& sdf_type)
{
    return 0;
};

//! Matter Access Type -> Data Quality
int map_matter_access(accessType& access, dataQualityType& dataQuality)
{
    //TODO: Can access be represented like this?
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(eventType& event, sdfEventType& sdfEvent)
{
    sdfEvent.label = event.name;
    sdfEvent.description = event.description;
    // access
    save_to_mapping("code", event.code);
    save_to_mapping("side", event.side);
    // priority
    for (eventFieldType& eventField : event.field){
        sdfEvent.sdfOutputData.sdfChoice.insert({});
    }
    return 0;
}

//! Matter Command -> sdfAction
int map_matter_command(commandType& command, sdfActionType& sdfAction)
{
    //TODO: As client and server are seperated, they have to be merged after processing all commands
    sdfAction.label = command.name;
    sdfAction.description = command.description;

    //sdfAction.sdfData.insert()
    // access

    //! Indicates that the command is a request command
    if (command.source == "client") {
        //! Map Command Arguments
        for (argType &arg: command.arg){
            dataQualityType dataQualities;
            //! Common qualities
            dataQualities.label = arg.name;
            dataQualities.description = arg.description;

            dataQualities.default_ = arg.default_;
            dataQualities.nullable = arg.isNullable;
            dataQualities.type = arg.type.name; //TODO: This needs mapping
            // arraylength
            // array
            save_to_mapping("introducedIn", command.introducedIn);
            //save_to_mapping("removedIn", command.removedIn);
            // length
            // presentIf
            // optional
            // fieldIf
            // countArg
            sdfAction.sdfInputData.sdfChoice.insert({arg.name, dataQualities});
        }
    }
    //! Indicates that the command only contains the returned values in response to a request
    if (command.source == "server"){
        //sdfAction.sdfOutputData.
        //TODO: The command output is in itself another command, they are matched via the response field
        //The response field contains the name of the responding command
        //We probably have to search for each reference to differentiate between request and response commands
        //Maybe they can be identified by the source they're coming from
        // -> Client : Request
        // -> Server : Response
    }


    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty)
{
    sdfProperty.label = attribute.name;
    sdfProperty.description = attribute.description;
    if (!attribute.optional){
        sdfProperty.sdfRequired; //TODO: Create a sdfRef here
    }

    sdfProperty.type = attribute.type; //TODO: This definitely needs mapping
    sdfProperty.default_ = attribute.default_;
    sdfProperty.minLength = attribute.min; //TODO: does this match?
    sdfProperty.maxLength = attribute.max; //TODO: does this match?
    sdfProperty.nullable = attribute.isNullable;
    // access
    save_to_mapping("code", attribute.code);
    save_to_mapping("define", attribute.define);
    save_to_mapping("introducedIn", attribute.introducedIn);
    // length
    save_to_mapping("manufacturerCode", attribute.manufacturerCode);
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    save_to_mapping("side", attribute.side);
    // array
    sdfProperty.readable = attribute.readable;
    sdfProperty.writable = attribute.writable;
    sdfProperty.observable = attribute.reportable; //TODO: Does this match
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& clusterNode)
{
    sdfObject.label = cluster.name;
    clusterNode.append_child(cluster.name.c_str());
    std::cout << "Cluster Name: " << cluster.name << std::endl;
    sdfObject.description = cluster.description;

    for (attributeType& attribute : cluster.attributes){
        sdfPropertyType sdfProperty;
        map_matter_attribute(attribute, sdfProperty);
        sdfObject.sdfProperty.insert({attribute.name, sdfProperty});
    }

    for (commandType& command : cluster.commands){
        sdfActionType sdfAction;
        map_matter_command(command, sdfAction);
        sdfObject.sdfAction.insert({command.name, sdfAction});
    }

    for (eventType& event : cluster.events){
        sdfEventType sdfEvent;
        map_matter_event(event, sdfEvent);
        sdfObject.sdfEvent.insert({event.name, sdfEvent});
    }
    return 0;
}

//! Matter Device -> SDF-Model
int map_matter_device(deviceType& device, sdfModelType& sdfModel, pugi::xml_node& deviceNode)
{

    //! Information Block
    sdfModel.infoBlock.title = device.name;
    std::cout << "Currently Mapping: " << device.name << std::endl;

    //! Append the device node to the tree
    deviceNode.append_child(device.name.c_str()).append_child("sdfObject");
    auto cluster_node = deviceNode.child(device.name.c_str()).child("sdfObject");

    //! Namespace Block
    sdfModel.namespaceBlock.namespaces.insert({"zcl", ""});
    sdfModel.namespaceBlock.defaultNamespace = "zcl";

    //! Definition Block
    sdfThingType sdfThing;
    sdfThing.label = device.name;

    //! Iterate through cluster definitions for the device
    for (auto cluster : device.clusters){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject, cluster_node);
        sdfThing.sdfObject.insert({cluster.name, sdfObject});
    }

    sdfModel.sdfThing = sdfThing;
    return 0;
}

//! For debug purposes, prints a visual representation of the tree
struct simple_walker: pugi::xml_tree_walker
{
    bool for_each(pugi::xml_node& node) override
    {
        for (int i = 0; i < depth(); ++i) std::cout << "--> "; // indentation

        std::cout << node.name() << std::endl;

        return true; // continue traversal
    }
};


//! Matter -> SDF
int map_matter_to_sdf(deviceType& device, sdfModelType& sdfModel, sdfMappingType& sdfMapping)
{
    //TODO: sdfMapping is currently unused
    pugi::xml_document referenceTree;
    referenceTree.append_child("#").append_child("sdfThing");
    auto device_node = referenceTree.child("#").child("sdfThing");
    //! Create a walker to visually represent the tree
    simple_walker walker;
    map_matter_device(device, sdfModel, device_node);
    //! Print the resulting tree
    referenceTree.traverse(walker);
    return 0;
}
