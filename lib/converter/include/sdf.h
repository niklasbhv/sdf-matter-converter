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
 * Structs which contain parsed information for sdf and functions to parse from json files.
 */

#ifndef SDF_MATTER_CONVERTER_SDF_H
#define SDF_MATTER_CONVERTER_SDF_H

#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include <variant>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

NLOHMANN_JSON_NAMESPACE_BEGIN

//! Template used to add to_json and from_json for the std::optional type in combination with std::monostate.
//! This type is used to represent null values.
template <> struct adl_serializer<std::optional<std::monostate>> {
    // Define to_json function for std::optional<std::monostate>
    static void to_json(ordered_json &j, const std::optional<std::monostate> &opt) {
        if (opt.has_value()) {
            j = nullptr;  // Serialize std::monostate as JSON null
        } else {
            j = nullptr;  // Serialize empty optional as JSON null
        }
    }

    // Define from_json function for std::optional<std::monostate>
    static void from_json(const ordered_json &j, std::optional<std::monostate> &opt) {
        if (!j.is_null()) {
            throw ordered_json::type_error::create(302, "type error: expected null", &j);
        }
        opt = std::monostate{};
    }
};

//! Template used to add to_json and from_json for the std::variant type.
// Try to set the value of type T into the variant data
// if it fails, do nothing
template <typename T, typename... Types>
void variant_from_json(const ordered_json& j, std::variant<Types...> &data)
{
    try {
        data = j.get<T>();
        if (j.is_number() and !j.is_number_float()) {
            if (j < (uint64_t ) 0) {
                data = j.get<int64_t>();
            }
        }
    } catch (...) {

    }
}

template <typename... Types> struct adl_serializer<std::variant<Types...>> {
    static void to_json(ordered_json& j, const std::variant<Types...> &v) {
        std::visit([&j](const auto &value) {
            j = value; // Serialize the current value to JSON
        }, v);
    }
    static void from_json(const ordered_json& j, std::variant<Types...> &data)
    {
        // Call variant_from_json for all types, only one will succeed
        (variant_from_json<Types>(j, data), ...);
    }
};

//! Template used to add to_json and from_json for the std::optional type.
template <typename T> struct adl_serializer<std::optional<T>> {
    static void to_json(ordered_json& j, const std::optional<T>& opt) {
        if (opt == std::nullopt) {
            j = nullptr;
        } else {
            // this will call adl_serializer<T>::to_json which will
            // find the free function to_json in T's namespace!
            j = *opt;
        }
    }

    static void from_json(const ordered_json& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        } else {
            // same as above, but with  adl_serializer<T>::from_json
            opt = j.template get<T>();
        }
    }
};

NLOHMANN_JSON_NAMESPACE_END

namespace sdf {

//! Type definition for the possible types contained in the sdf-mapping
typedef std::variant<uint64_t, int64_t, double, std::string, bool, json> MappingValue;


//! Struct which contains common quality information.
struct CommonQuality {
    std::string description;
    std::string label;
    std::string comment;
    std::string sdf_ref;
    std::list<std::string> sdf_required;
};

//! Data quality struct definition.
struct DataQuality;

//! Type definition for sdfChoice.
typedef std::unordered_map<std::string, DataQuality> SdfChoice;

//! Type definition for sdfData.
typedef std::unordered_map<std::string, DataQuality> SdfData;

//! JSO-Item Type definition.
struct JsoItem {
    //! General qualities
    std::string sdf_ref;
    std::string description;
    std::string comment;
    //! Either number, string, boolean, integer or object
    std::string type;
    SdfChoice sdf_choice;
    std::list<std::string> enum_;
    //! Number and Integer qualities
    std::optional<std::variant<double, int64_t, uint64_t>> minimum;
    std::optional<std::variant<double, int64_t, uint64_t>> maximum;
    //! String qualities
    std::optional<uint64_t> min_length;
    std::optional<uint64_t> max_length;
    //! Either date-time, date, time, uri, uri-reference or uuid
    std::string format;
    //! Object qualities
    std::unordered_map<std::string, DataQuality> properties;
    std::list<std::string> required;
};

//! Type definition for array items.
    typedef std::variant<uint64_t, int64_t , double, std::string, bool> ArrayItem;

//! Type definition for const and default fields.
    typedef std::variant<uint64_t, int64_t , double, std::string, bool, std::list<ArrayItem>, std::optional<std::monostate>> VariableType;

//! Struct which contains data quality information.
struct DataQuality : CommonQuality {
    //! General qualities
    //! Either number, string, boolean, integer, array or object
    std::string type;
    SdfChoice sdf_choice;
    std::list<std::string> enum_;
    std::optional<VariableType> const_;
    std::optional<VariableType> default_;
    //! Number and Integer qualities
    std::optional<std::variant<double, int64_t, uint64_t>> minimum;
    std::optional<std::variant<double, int64_t, uint64_t>> maximum;
    std::optional<std::variant<double, int64_t, uint64_t>> exclusive_minimum;
    std::optional<std::variant<double, int64_t, uint64_t>> exclusive_maximum;
    std::optional<std::variant<double, int64_t, uint64_t>> multiple_of;
    //! String qualities
    std::optional<uint64_t> min_length;
    std::optional<uint64_t> max_length;
    std::string pattern;
    //! Either date-time, date, time, uri, uri-reference or uuid
    std::string format;
    //! Array qualities
    std::optional<uint64_t> min_items;
    std::optional<uint64_t> max_items;
    std::optional<bool> unique_items;
    std::optional<JsoItem> items;
    //! Object qualities
    SdfData properties;
    std::list<std::string> required;
    //! Additional qualities
    std::string unit;
    std::optional<bool> nullable;
    //! Either byte-string or unix-time
    std::string sdf_type;
    std::string content_format;
};

//! Struct which contains sdfEvent information.
struct SdfEvent : CommonQuality {
    std::optional<DataQuality> sdf_output_data;
    SdfData sdf_data;
};

//! Struct which contains sdfAction information.
struct SdfAction : CommonQuality {
    std::optional<DataQuality> sdf_input_data;
    std::optional<DataQuality> sdf_output_data;
    SdfData sdf_data;
};

//! Struct which contains sdfProperty information.
struct SdfProperty : DataQuality {
    std::optional<bool> readable;
    std::optional<bool> writable;
    std::optional<bool> observable;
};

//! Struct which contains sdfObject information.
struct SdfObject : CommonQuality {
    std::unordered_map<std::string, SdfProperty> sdf_property;
    std::unordered_map<std::string, SdfAction> sdf_action;
    std::unordered_map<std::string, SdfEvent> sdf_event;
    SdfData sdf_data;
    //! Array definition qualities
    std::optional<uint> min_items;
    std::optional<uint> max_items;
};

//! Struct which contains sdfThing information.
struct SdfThing : CommonQuality{
    std::unordered_map<std::string, SdfThing> sdf_thing;
    std::unordered_map<std::string, SdfObject> sdf_object;
    std::unordered_map<std::string, SdfProperty> sdf_property;
    std::unordered_map<std::string, SdfAction> sdf_action;
    std::unordered_map<std::string, SdfEvent> sdf_event;
    SdfData sdf_data;
    //! Array definition qualities
    std::optional<uint> min_items;
    std::optional<uint> max_items;
};

//! Struct which contains namespace block information.
struct NamespaceBlock {
    std::unordered_map<std::string, std::string> namespaces;
    std::string default_namespace;
};

//! Struct which contains information block information.
struct InformationBlock {
    std::string title;
    std::string description;
    std::string version;
    std::string modified;
    std::string copyright;
    std::string license;
    std::string features;
    std::string comment;
};

//! Struct which contains sdf-model information.
struct SdfModel {
    std::optional<InformationBlock> information_block;
    std::optional<NamespaceBlock> namespace_block;
    std::unordered_map<std::string, SdfThing> sdf_thing;
    std::unordered_map<std::string, SdfObject> sdf_object;
};

//! Struct which contains sdf-mapping information.
struct SdfMapping {
    std::optional<InformationBlock> information_block;
    std::optional<NamespaceBlock> namespace_block;
    std::unordered_map<std::string, std::unordered_map<std::string, MappingValue>> map;
};

//! @brief Parse a sdf-model.
//!
//! This functions parses a sdf-model into an object.
//!
//! @param sdf_model_json The input sdf-model.
//! @return The parsed sdf-model.
SdfModel ParseSdfModel(json& sdf_model_json);

//! @brief Parse a sdf-mapping.
//!
//! This function parses a sdf-mapping into an object.
//!
//! @param sdf_mapping_json The input sdf-mapping.
//! @return The parsed sdf-mapping.
SdfMapping ParseSdfMapping(json& sdf_mapping_json);


//! @brief Serialize a sdf-model.
//!
//! This function serializes a sdf-model into json.
//!
//! @param sdf_model The input sdf-model.
//! @return The serialized sdf-model.
json SerializeSdfModel(const SdfModel& sdf_model);

//! @bried Serialize a sdf-mapping.
//!
//! This function serializes a sdf-mapping into json.
//!
//! @param sdf_mapping The input sdf-mapping.
//! @return The serialized sdf-mapping.
json SerializeSdfMapping(const SdfMapping& sdf_mapping);

} // namespace sdf

#endif //SDF_MATTER_CONVERTER_SDF_H
