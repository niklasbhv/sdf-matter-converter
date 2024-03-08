//
// Created by niklas on 05.03.24.
//

#ifndef SDF_H
#define SDF_H

// TODO: A lot of these qualities are n-ary, but currently in most cases only one object is possible

#include <string>
#include <map>
#include <list>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct sdfCommonType {
    std::string description;
    std::string label;
    std::string $comment;
    // sdfRef
    // sdfRequired
};

struct sdfDataType {
    sdfCommonType commonQualities;
    std::string unit;
    bool nullable;
    std::string contentFormat;
    std::string sdfType;
    std::map<std::string, sdfDataType> sdfChoice;
    // std::string enum[];
};

struct sdfEventType {
    sdfCommonType commonQualities;
    std::map<std::string, sdfDataType> sdfOutputData;
    // sdfData
};

struct sdfActionType {
    sdfCommonType commonQualities;
    std::map<std::string, sdfDataType> sdfInputData;
    std::map<std::string, sdfDataType> sdfOutputData;
    // sdfData
};

// TODO:: Currently these default to false, check if there should be a default
struct sdfPropertyType {
    sdfDataType dataQualities;
    bool readable = false;
    bool writable = false;
    bool observable = false;
};

struct sdfObjectType {
    sdfCommonType commonQualities;
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    std::map<std::string, sdfDataType> sdfData;
    int minItems;
    int maxItems;
};

struct sdfThingType {
    sdfCommonType commonQualities;
    // sdfThing
    sdfObjectType sdfObject;
    sdfPropertyType sdfProperty;
    sdfActionType sdfAction;
    sdfEventType sdfEvent;
    sdfDataType sdfData;
    int minItems;
    int maxItems;
};

struct namespaceType {
    std::map<std::string, std::string> namespaces;
    std::string defaultNamespace;
};

struct infoBlockType{
    std::string title;
    std::string description;
    std::string version;
    std::string modified;
    std::string copyright;
    std::string license;
    std::string features;
    std::string $comment;
};

struct sdfModelType{
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, sdfThingType>sdfThings;
    std::map<std::string, sdfObjectType> sdfObjects;
};

int parseSdfModel(const json& sdf_model, std::list<sdfModelType>& sdfModelList);
int parseSdfMapping(const json& sdf_mapping);


#endif //SDF_H
