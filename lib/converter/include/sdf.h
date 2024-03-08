#ifndef SDF_H
#define SDF_H

#include <string>
#include <map>
#include <list>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct sdfCommonType {
    std::string description;
    std::string label;
    std::string $comment;
    std::string sdfRef;
    std::list<std::string> sdfRequired;
};

struct sdfDataType {
    //TODO: Additional fields get pulled from jsonschema
    sdfCommonType commonQualities;
    std::string unit;
    bool nullable;
    std::string contentFormat;
    std::string sdfType;
    std::map<std::string, sdfDataType> sdfChoice;
    std::list<std::string> enm;
};

struct sdfEventType {
    sdfCommonType commonQualities;
    sdfDataType sdfOutputData;
    std::map<std::string, sdfDataType> sdfData; //TODO: Check if this is sufficient
};

struct sdfActionType {
    sdfCommonType commonQualities;
    sdfDataType sdfInputData; //TODO: Check if this is sufficient
    sdfDataType sdfOutputData; //TODO: Check if this is sufficient
    std::map<std::string, sdfDataType> sdfData; //TODO: Check if this is sufficient
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
    //! It's currently not planed to allow for nested sdfThings as they
    //! wouldn't really be able to be translated into Matter
    std::map<std::string, sdfObjectType> sdfObject;
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    std::map<std::string, sdfDataType> sdfData;
    int minItems;
    int maxItems;
};

struct namespaceType {
    std::map<std::string, std::string> namespaces;
    std::string defaultNamespace;
};

struct infoBlockType {
    std::string title;
    std::string description;
    std::string version;
    std::string modified;
    std::string copyright;
    std::string license;
    std::string features;
    std::string $comment;
};

struct sdfModelType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, sdfThingType>sdfThings;
    std::map<std::string, sdfObjectType> sdfObjects;
};

struct sdfMappingType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, std::string> map;
};

int parseSdfModel(const json& sdf_model, sdfModelType& sdfModel);
int parseSdfMapping(const json& sdf_mapping, sdfMappingType& sdfMapping);

#endif //SDF_H
