#include "mapping.h"
#include "matter.h"
#include "sdf.h"

int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType)
{
    return 0;
}

int map_sdf_thing(sdfThingType& sdfThing, deviceType& device)
{
    return 0;
}

int map_sdf_object(sdfObjectType& sdfObject, clusterType& cluster)
{
    return 0;
}

int map_sdf_property(sdfPropertyType& sdfProperty, attributeType& attribute)
{
    return 0;
}

int map_sdf_action(sdfActionType& sdfAction, commandType& command)
{
    return 0;
}

int map_sdf_event(sdfEventType& sdfEvent, eventType& event)
{
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(eventType& event, sdfEventType& sdfEvent)
{
    return 0;
}

//! Matter Command -> sdfAction
int map_matter_command(commandType& command, sdfActionType& sdfAction)
{
    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty)
{
    sdfCommonType commonQualities;
    commonQualities.label = attribute.name;
    commonQualities.description = attribute.description;
    if (!attribute.optional){
        commonQualities.sdfRequired; //TODO: Create a sdfRef here
    }

    // access

    sdfDataType dataQualities;
    dataQualities.commonQualities = commonQualities;

    // attribute.code -> Mapping file
    // attribute.deflt
    // attribute.define -> Mapping file
    // introducedIn -> Mapping file
    // length
    // manufacturerCode -> Mapping file
    // max
    // min
    // reportMaxInterval
    // reportMinInterval
    // side

    dataQualities.sdfType = attribute.type; //TODO: This might need mapping of the types
    sdfProperty.readable = attribute.readable;
    sdfProperty.writable = attribute.writable;
    sdfProperty.observable = attribute.reportable; //TODO: Does this match?

    // array

    dataQualities.nullable = attribute.isNullable;
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject)
{
    sdfCommonType commonQualities;
    commonQualities.label = cluster.name;
    commonQualities.description = cluster.description;
    sdfObject.commonQualities = commonQualities;

    std::map<std::string, sdfPropertyType> sdfPropertyMap;
    for (attributeType& attribute : cluster.attributes){
        sdfPropertyType sdfProperty;
        map_matter_attribute(attribute, sdfProperty);
        sdfPropertyMap.insert({attribute.name, sdfProperty});
    }
    sdfObject.sdfProperty = sdfPropertyMap;

    std::map<std::string, sdfActionType> sdfActionMap;
    for (commandType& command : cluster.commands){
        sdfActionType sdfAction;
        map_matter_command(command, sdfAction);
        sdfActionMap.insert({command.name, sdfAction});
    }
    sdfObject.sdfAction = sdfActionMap;

    std::map<std::string, sdfEventType> sdfEventMap;
    for (eventType& event : cluster.events){
        sdfEventType sdfEvent;
        map_matter_event(event, sdfEvent);
        sdfEventMap.insert({event.name, sdfEvent});
    }
    sdfObject.sdfEvent = sdfEventMap;
    return 0;
}

//! Matter Device -> SDF-Model
int map_matter_device(deviceType& device, sdfModelType& sdfModel)
{
    //! Information Block
    infoBlockType infoBlock;
    infoBlock.title = device.name;
    sdfModel.infoBlock = infoBlock;

    //! Namespace Block
    namespaceType namespaceBlock;
    sdfModel.namespaceBlock = namespaceBlock;

    //! Definition Block
    sdfThingType sdfThing;
    sdfCommonType commonQualities;
    commonQualities.label = device.name;
    sdfThing.commonQualities = commonQualities;

    std::map<std::string, sdfObjectType> sdfObjectMap;
    for (clusterType& cluster : device.clusters){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject);
        sdfObjectMap.insert({cluster.name, sdfObject});
    }
    sdfThing.sdfObject = sdfObjectMap;
    return 0;
}

//! Matter -> SDF
int matter_to_sdf(deviceType& device, clusterType& cluster)
{
    return 0;
}
