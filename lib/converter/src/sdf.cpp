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

int parseDefinitionBlock(const json& definition_json, definitionType& definitionBlock)
{
    return 0;
}

int parseNamespaceBlock(const json& namespace_json, namespaceType& nsp)
{
    for (auto nsp_item : namespace_json.items()) {
        nsp.namespaces.insert({nsp_item.key(), nsp_item.value()});
    }
    return 0;
}

int parseInfoBlock(const json& info_json, infoBlockType& infoBlock)
{
    info_json.at("title").get_to(infoBlock.title);
    info_json.at("description").get_to(infoBlock.description);
    info_json.at("version").get_to(infoBlock.version);
    info_json.at("modified").get_to(infoBlock.modified);
    info_json.at("copyright").get_to(infoBlock.copyright);
    info_json.at("license").get_to(infoBlock.license);
    info_json.at("features").get_to(infoBlock.features);
    info_json.at("$comment").get_to(infoBlock.$comment);
    return 0;
}

int parseSdfModel(const json& sdf_model, std::list<sdfModelType>& sdfModelList)
{
    sdfModelType sdfModel;
    if (sdf_model.contains("info")) {
        infoBlockType infoBlock;
        parseInfoBlock(sdf_model.at("info"), infoBlock);
        sdfModel.infoBlock = infoBlock;
    }

    if (sdf_model.contains("namespace")) {
        if (sdf_model.contains("defaultNamespace")) {
            namespaceType nsp;
            parseNamespaceBlock(sdf_model.at("namespace"), nsp);
            sdf_model.at("defaultNamespace").get_to(nsp.defaultNamespace);
            sdfModel.namespaceBlock = nsp;
        }
    }

    definitionType definitionBlock;
    //! Does the SDF-Model contain a sdfThing?
    if (sdf_model.contains("sdfThing")){
        parseDefinitionBlock(sdf_model.at("sdfThing"), definitionBlock);
    }
    //! If not, does the SDF-Model contain a sdfObject?
    else if (sdf_model.contains("sdfObject")){
        parseDefinitionBlock(sdf_model.at("sdfObject"), definitionBlock);
    }
    else {
        return -1;
    }
    return 0;
}

int parseSdfMapping(const json& sdf_mapping)
{
    return 0;
}
