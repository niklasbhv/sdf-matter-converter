#ifndef SDF_H
#define SDF_H

#include <string>
#include <map>
#include <list>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

struct commonQualityType {
    std::string description;
    std::string label;
    std::string $comment;
    std::string sdfRef;
    std::list<std::string> sdfRequired;
};

struct dataQualityType;

struct jsoItemType {
    std::string sdfRef;
    std::string description;
    std::map<std::string, dataQualityType> sdfChoice;
    std::list<std::string> enum_;
    std::string type;
    int minimum; // number
    int maximum; // number
    std::string format;
    uint minLength;
    uint maxLength;
};

struct dataQualityType : commonQualityType {
    //! JSON schema qualities
    std::string type; // number / string / boolean / integer / array
    std::map<std::string, dataQualityType> sdfChoice;
    std::list<std::string> enum_;
    std::string const_;
    std::string default_;
    int minimum; // number
    int maximum; // number
    int exclusiveMinimum; // number
    int exclusiveMaximum; // number
    int multipleOf; // number
    //! Text string constraints
    uint minLength;
    uint maxLength;
    std::string pattern;
    std::string format; // date-time / date / time / uri / uri-reference / uuid
    //! Array constraints
    uint minItems;
    uint maxItems;
    bool uniqueItems;
    jsoItemType items;
    //! Additional qualities
    std::string unit;
    bool nullable;
    std::string sdfType; // byte-string / unix-time
    std::string contentFormat;
    //! Defined in the specification
};

typedef std::map<std::string, dataQualityType> sdfDataType;
typedef std::map<std::string, dataQualityType> sdfChoiceType;

struct sdfEventType : commonQualityType {
    dataQualityType sdfOutputData;
    sdfDataType sdfData;
};

struct sdfActionType : commonQualityType {
    dataQualityType sdfInputData;
    dataQualityType sdfOutputData;
    sdfDataType sdfData;
};

struct sdfPropertyType : dataQualityType {
    bool readable = true;
    bool writable = true;
    bool observable = true;
};

struct sdfObjectType : commonQualityType {
    //! Paedata qualities
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    sdfDataType sdfData;
    //! Array definition qualities
    uint minItems;
    uint maxItems;
};

struct sdfThingType : commonQualityType{
    //! It's currently not planed to allow for nested sdfThings as they
    //! wouldn't really be able to be translated into Matter
    std::map<std::string, sdfObjectType> sdfObject;
    //! Paedata qualities
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    sdfDataType sdfData;
    //! Array definition qualities
    uint minItems;
    uint maxItems;
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
    //! Either a single sdfThing or a single sdfObject are allowed per model
    std::optional<sdfThingType>sdfThing;
    std::optional<sdfObjectType> sdfObject;
};

struct sdfMappingType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, std::string> map;
};

int parseSdfModel(const json& sdf_model, sdfModelType& sdfModel);
int parseSdfMapping(const json& sdf_mapping, sdfMappingType& sdfMapping);

#endif //SDF_H
