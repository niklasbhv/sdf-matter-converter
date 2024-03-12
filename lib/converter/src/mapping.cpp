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
    //TODO: As client and server are seperated, they have to be merged after processing all commands
    commonQualityType commonQualities;
    commonQualities.label = command.name;
    commonQualities.description = command.description;
    sdfAction.commonQualities = commonQualities;

    sdfDataType sdfData;



    //sdfAction.sdfData = sdfData;
    //! Indicates that the command is a request command
    if (command.source == "client") {
        //! Map Command Arguments
        dataQualityType sdfInputData;
        for (argType &arg: command.arg) {
            // arraylength
            // array
            // deflt
            // introducedIn -> Mapping file
            // removedIn -> Mapping file
            // length
            // presentIf
            // optional
            // fieldIf
            // countArg
            //sdfInputData.commonQualities.label = arg.name;
            //sdfInputData.commonQualities.description = arg.description;
            //sdfInputData.sdfType = arg.type.name; //TODO: This probably needs mapping
            //sdfInputData.nullable = arg.isNullable;
        }
        sdfAction.sdfInputData = sdfInputData;
    }
    //! Indicates that the command only contains the returned values in response to a request
    if (command.source == "server"){
        dataQualityType sdfOutputData;
        //TODO: The command output is in itself another command, they are matched via the response field
        //The response field contains the name of the responding command
        //We probably have to search for each reference to differentiate between request and response commands
        //Maybe they can be identified by the source they're coming from
        // -> Client : Request
        // -> Server : Response

        sdfAction.sdfOutputData = sdfOutputData;
    }


    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty)
{
    commonQualityType commonQualities;
    commonQualities.label = attribute.name;
    commonQualities.description = attribute.description;
    if (!attribute.optional){
        commonQualities.sdfRequired; //TODO: Create a sdfRef here
    }

    dataQualityType dataQualities;
    dataQualities.commonQualities = commonQualities;
    sdfProperty.dataQualities.type = attribute.type; //TODO: This definitely needs mapping
    sdfProperty.dataQualities.deflt = attribute.deflt;
    sdfProperty.dataQualities.minLength = attribute.min; //TODO: does this match?
    sdfProperty.dataQualities.maxLength = attribute.max; //TODO: does this match?
    sdfProperty.dataQualities.nullable = attribute.isNullable;
    // access
    // code -> Mapping file
    // define -> Mapping file
    // introducedIn -> Mapping file
    // length
    // manufacturerCode -> Mapping file
    // reportMaxInterval
    // reportMinInterval
    // reportableChange
    // side -> Mapping file
    // array
    sdfProperty.readable = attribute.readable;
    sdfProperty.writable = attribute.writable;
    sdfProperty.observable = attribute.reportable; //TODO: Does this match
    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(clusterType& cluster, sdfObjectType& sdfObject)
{
    commonQualityType commonQualities;
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
    commonQualityType commonQualities;
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
