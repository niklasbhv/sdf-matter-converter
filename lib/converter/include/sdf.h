/**
 *  Copyright 2024 Niklas Meyer
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
#include <variant>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

/**
 * Template used to add to_json and from_json for the std::optional type.
 */
NLOHMANN_JSON_NAMESPACE_BEGIN
template <typename T> struct adl_serializer<std::optional<T>> {
    static void to_json(json& j, const std::optional<T>& opt) {
        if (opt == std::nullopt) {
            j = nullptr;
        } else {
            // this will call adl_serializer<T>::to_json which will
            // find the free function to_json in T's namespace!
            j = *opt;
        }
    }

    static void from_json(const json& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        } else {
            // same as above, but with  adl_serializer<T>::from_json
            opt = j.template get<T>();
        }
    }
};
NLOHMANN_JSON_NAMESPACE_END

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
 * Type definition for sdfChoice.
 */
typedef std::map<std::string, dataQualityType> sdfChoiceType;

/**
 * Type definition for sdfData.
 */
typedef std::map<std::string, dataQualityType> sdfDataType;

/**
 * JSO-Item Type definition.
 */
struct jsoItemType {
    std::string sdfRef;
    std::string description;
    std::string $comment;
    std::string type; // number / string / boolean / integer / object
    sdfChoiceType sdfChoice;
    std::list<std::string> enum_;
    std::optional<int> minimum;
    std::optional<int> maximum;
    std::string format;
    std::optional<uint> minLength;
    std::optional<uint> maxLength;
    std::list<std::string> required;
    sdfDataType properties;
};

/**
 * Type definition for array items.
 */
typedef std::variant<uint64_t, int64_t , double, std::string, bool> arrayItemType;

/**
 * Type definition for const and default fields.
 */
typedef std::variant<uint64_t, int64_t , double, std::string, bool, std::list<arrayItemType>> variableType;

/**
 * Struct which contains data quality information.
 */
struct dataQualityType : commonQualityType {
    // General qualities
    std::string type; // number / string / boolean / integer / array / object
    sdfChoiceType sdfChoice;
    std::list<std::string> enum_;
    std::optional<variableType> const_;
    std::optional<variableType> default_;
    // Number and Integer qualities
    std::optional<std::variant<double, int64_t, uint64_t>> minimum;
    std::optional<std::variant<double, int64_t, uint64_t>> maximum;
    std::optional<std::variant<double, int64_t, uint64_t>> exclusiveMinimum;
    std::optional<std::variant<double, int64_t, uint64_t>> exclusiveMaximum;
    std::optional<std::variant<double, int64_t, uint64_t>> multipleOf;
    // String qualities
    std::optional<uint> minLength;
    std::optional<uint> maxLength;
    std::string pattern;
    std::string format; // date-time / date / time / uri / uri-reference / uuid
    // Array qualities
    std::optional<uint> minItems;
    std::optional<uint> maxItems;
    std::optional<bool> uniqueItems;
    std::optional<jsoItemType> items;
    // Object qualities
    sdfDataType properties;
    std::list<std::string> required;
    // Additional qualities
    std::string unit;
    std::optional<bool> nullable;
    std::string sdfType; // byte-string / unix-time
    std::string contentFormat;
};

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
    std::optional<bool> readable;
    std::optional<bool> writable;
    std::optional<bool> observable;
};

/**
 * Struct which contains sdfObject information.
 */
struct sdfObjectType : commonQualityType {
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    sdfDataType sdfData;
    // Array definition qualities
    std::optional<uint> minItems;
    std::optional<uint> maxItems;
};

/**
 * Struct which contains sdfThing information.
 */
struct sdfThingType : commonQualityType{
    std::map<std::string, sdfThingType> sdfThing;
    std::map<std::string, sdfObjectType> sdfObject;
    std::map<std::string, sdfPropertyType> sdfProperty;
    std::map<std::string, sdfActionType> sdfAction;
    std::map<std::string, sdfEventType> sdfEvent;
    sdfDataType sdfData;
    // Array definition qualities
    std::optional<uint> minItems;
    std::optional<uint> maxItems;
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
    std::map<std::string, sdfThingType> sdfThing;
    std::map<std::string, sdfObjectType> sdfObject;
};

/**
 * Struct which contains sdf-mapping information.
 */
struct sdfMappingType {
    infoBlockType infoBlock;
    namespaceType namespaceBlock;
    std::map<std::string, std::map<std::string, std::string>> map;
};

/**
 * @brief Parse a sdf-model.
 *
 * This functions parses a sdf-model into an object.
 *
 * @param sdf_model_json The input sdf-model.
 * @param sdfModel The resulting object.
 * @return 0 on success, negative on failure.
 */
int parseSdfModel(json& sdf_model_json, sdfModelType& sdfModel);

/**
 * @brief Parse a sdf-mapping.
 *
 * This function parses a sdf-mapping into an object.
 *
 * @param sdf_mapping_json The input sdf-mapping.
 * @param sdfMapping The resulting object.
 * @return 0 on success, negative on failure.
 */
int parseSdfMapping(json& sdf_mapping_json, sdfMappingType& sdfMapping);


/**
 * @brief Serialize a sdf-model.
 *
 * This function serializes a sdf-model into json.
 *
 * @param sdfModel The input sdf-model.
 * @param sdf_model_json The resulting json.
 * @return 0 on success, negative on failure.
 */
int serializeSdfModel(const sdfModelType& sdfModel, json& sdf_model_json);

/**
 * @bried Serialize a sdf-mapping.
 *
 * This function serializes a sdf-mapping into json.
 *
 * @param sdfMapping The input sdf-mapping.
 * @param sdf_mapping_json The resulting json.
 * @return 0 on success, negative on failure.
 */
int serializeSdfMapping(const sdfMappingType& sdfMapping, json& sdf_mapping_json);

#endif //SDF_H
