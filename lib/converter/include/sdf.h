/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Structs to contain parsed information for sdf and functions to parse from json files.
 */
#ifndef SDF_H
#define SDF_H

#include <string>
#include <map>
#include <list>
#include <nlohmann/json.hpp>
#include <optional>

//TODO: How do we check if uint or bool are empty?

using json = nlohmann::json;

/**
 * Struct which contains common quality information.
 */
struct commonQualityType {
    std::string description;
    std::string label;
    std::string $comment;
    std::string sdfRef;
    std::list<std::string> sdfRequired;
};

/**
 * Data quality struct definition.
 */
struct dataQualityType;

/**
 * Struct which contains jso information.
 */
struct jsoItemType {
    std::string sdfRef;
    std::string description;
    std::map<std::string, dataQualityType> sdfChoice;
    std::list<std::string> enum_;
    std::string type;
    int minimum = -1; // number
    int maximum = -1; // number
    std::string format;
    uint minLength;
    uint maxLength;
};

/**
 * Struct which contains data quality information.
 */
struct dataQualityType : commonQualityType {
    //! JSON schema qualities
    std::string type; // number / string / boolean / integer / array
    std::map<std::string, dataQualityType> sdfChoice;
    std::list<std::string> enum_;
    std::string const_;
    std::string default_;
    int minimum = -1; // number
    int maximum = -1; // number
    int exclusiveMinimum = -1; // number
    int exclusiveMaximum = -1; // number
    int multipleOf = -1; // number
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

/**
 * Type definition for sdfData.
 */
typedef std::map<std::string, dataQualityType> sdfDataType;

/**
 * Type definition for sdfChoice.
 */
typedef std::map<std::string, dataQualityType> sdfChoiceType;

/**
 * Struct which contains sdfEvent information.
 */
struct sdfEventType : commonQualityType {
    dataQualityType sdfOutputData;
    sdfDataType sdfData;
};

/**
 * Struct which contains sdfAction information.
 */
struct sdfActionType : commonQualityType {
    dataQualityType sdfInputData;
    dataQualityType sdfOutputData;
    sdfDataType sdfData;
};

/**
 * Struct which contains sdfProperty information.
 */
struct sdfPropertyType : dataQualityType {
    bool readable = true;
    bool writable = true;
    bool observable = true;
};

/**
 * Struct which contains sdfObject information.
 */
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

/**
 * Struct which contains sdfThing information.
 */
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

/**
 * Struct which contains namespace block information.
 */
struct namespaceType {
    std::map<std::string, std::string> namespaces;
    std::string defaultNamespace;
};

/**
 * Struct which contains information block information.
 */
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

/**
 * Struct which contains sdf-model information.
 */
struct sdfModelType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    //! Either a single sdfThing or a single sdfObject are allowed per model
    std::optional<sdfThingType>sdfThing;
    std::optional<sdfObjectType> sdfObject;
};

/**
 * Struct which contains sdf-mapping information.
 */
struct sdfMappingType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, std::string> map;
};

/**
 * @brief Parse a sdf-model.
 *
 * This functions parses a sdf-model into an object.
 *
 * @param sdf_model The input sdf-model.
 * @param sdfModel The resulting object.
 * @return 0 on success, negative on failure.
 */
int parseSdfModel(const json& sdf_model, sdfModelType& sdfModel);

/**
 * @brief Parse a sdf-mapping.
 *
 * This function parses a sdf-mapping into an object.
 *
 * @param sdf_mapping The input sdf-mapping.
 * @param sdfMapping The resulting object.
 * @return 0 on success, negative on failure.
 */
int parseSdfMapping(const json& sdf_mapping, sdfMappingType& sdfMapping);

#endif //SDF_H
