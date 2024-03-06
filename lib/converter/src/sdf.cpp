//
// Created by niklas on 05.03.24.
//

#include "sdf.h"
#include "matter.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int parseSdfEvent(const json& sdf_model)
{
    return 0;
}

int parseSdfAction(const json& sdf_model)
{
    return 0;
}

int parseSdfProperty(const json& sdf_model)
{
    return 0;
}

int parseSdfObject(const json& sdf_model)
{
    return 0;
}

int parseSdfThing(const json& sdf_model)
{
    return 0;
}

int parseDefinitionBlock(const json& sdf_model)
{
    //! Does the SDF-Model contain a sdfThing?
    if(sdf_model.contains("sdfThing")){

    }
        //! If not, does the SDF-Model contain a sdfObject?
    else if(sdf_model.contains("sdfObject")){
        //TODO: Does this break after we leave sdfObject?
        for (auto sdf_object_it = sdf_model.find("sdfObject"); sdf_object_it != sdf_model.end(); sdf_object_it++) {
            //matter_device.clusters.push_back(parseSdfObject(*sdf_object_it));
        }
    }
        //! If no sdfThing and no sdfObject is present, there's something wrong
        //TODO: Do we eben have to check this?
    else {
        return -1;
    }
    return 0;
}

int parseNamespaceBlock(const json& sdf_model)
{
    return 0;
}

int parseInfoBlock(const json& sdf_model)
{
    return 0;
}

int parseSdfModel(const json& sdf_model, std::list<sdfModelType>& sdfModelList)
{
    parseInfoBlock(sdf_model);
    parseNamespaceBlock(sdf_model);
    parseDefinitionBlock(sdf_model);
    return 0;
}

int parseSdfMapping(const json& sdf_mapping)
{
    return 0;
}
