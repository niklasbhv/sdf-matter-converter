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
    // code -> Mapping file
    // side -> Mapping file
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
            // introducedIn -> Mapping file
            // removedIn -> Mapping file
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
    sdfObject.label = cluster.name;
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
int map_matter_device(deviceType& device, sdfModelType& sdfModel)
{
    //! Information Block
    sdfModel.infoBlock.title = device.name;

    //! Namespace Block
    sdfModel.namespaceBlock.namespaces.insert({"zcl", ""});
    sdfModel.namespaceBlock.defaultNamespace = "zcl";

    //! Definition Block
    sdfThingType sdfThing;
    sdfThing.label = device.name;

    for (clusterType& cluster : device.clusters){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject);
        sdfThing.sdfObject.insert({cluster.name, sdfObject});
    }
    return 0;
}

//! Matter -> SDF
int map_matter_to_sdf(deviceType& device, std::list<clusterType>& clusterList)
{
    sdfModelType sdfModel;
    map_matter_device(device, sdfModel);
    //! Iterate through clusters
    std::list<sdfObjectType> sdfObjectList;
    for (auto cluster : clusterList){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject);
        sdfObjectList.push_back(sdfObject);
    }
    return 0;
}
