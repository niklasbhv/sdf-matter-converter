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

//! Matter -> SDF
int matter_to_sdf(deviceType& device, clusterType& cluster)
{
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject)
{
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

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty)
{
    return 0;
}

//! Matter Command -> sdfAction
int map_matter_command(commandType& command, sdfActionType& sdfAction)
{
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(eventType& event, sdfEventType& sdfEvent)
{
    return 0;
}
