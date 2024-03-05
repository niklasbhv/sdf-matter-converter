//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;



//
// Functions responsible for the SDF -> Matter conversion
//

clusterType mapSdfObject(const json& sdf_model)
{

    return clusterType {};
}

int mapSdfThing()
{
    return 0;
}

int parseDefinitionBlock(const json& sdf_model, deviceType& matter_device)
{
    //! Does the SDF-Model contain a sdfThing?
    if(sdf_model.contains("sdfThing")){

    }
        //! If not, does the SDF-Model contain a sdfObject?
    else if(sdf_model.contains("sdfObject")){
        //TODO: Does this break after we leave sdfObject?
        for (auto sdf_object_it = sdf_model.find("sdfObject"); sdf_object_it != sdf_model.end(); sdf_object_it++) {
            matter_device.clusters.push_back(mapSdfObject(*sdf_object_it));
        }
    }
        //! If no sdfThing and no sdfObject is present, there's something wrong
        //TODO: Do we eben have to check this?
    else {
        return -1;
    }
    return 0;
}

int parseNamespaceBlock()
{
    return 0;
}

int parseInfoBlock(const json& sdf_model, deviceType& matter_device)
{
    matter_device.name = "";
    matter_device.domain = "SDF";
    //matter_device.typeName = sdf_model.infoBlock.title;
    matter_device.profileId = "0";
    matter_device.deviceId = "0";
    return 0;
}
