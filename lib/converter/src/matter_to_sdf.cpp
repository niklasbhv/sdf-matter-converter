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

#include <iostream>
#include <set>
#include <limits>
#include "matter_to_sdf.h"
#include "mapping.h"

//! This is a global pointer to the quality name current node
//! This is designed to point at the top level sdf element like
//! for example the `sdfThing` node, not a specific sdfThing
static ReferenceTreeNode* current_quality_name_node = nullptr;

//! This is a global pointer to the given name current node
//! This is designed to point at the given name of an element
//! for example the `OnOff` node, not a top level sdf element
static ReferenceTreeNode* current_given_name_node = nullptr;

//! Set containing the supported features
//! Contains their short code for usage with conformance's
static std::set<std::string> supported_features;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
static std::list<std::string> sdf_required_list;

//! Location of the sdfData for the currently mapped structure
static std::string sdf_data_location;

namespace matter {

//! Function used to convert a conformance object into a JSON structure
void to_json(json& j, const Conformance& conformance) {
    if (conformance.mandatory) {
        j = json{{"mandatoryConform", conformance.condition}};
    } else if (conformance.optional) {
        if (!conformance.choice.empty()) {
            json choice_json;
            choice_json["choice"] = conformance.choice;
            if (conformance.choice_more.has_value()) {
                choice_json["more"] = conformance.choice_more.value();
            }
            choice_json.merge_patch(conformance.condition);
            j = json{{"optionalConform", choice_json}};
        } else {
            j = json{{"optionalConform", conformance.condition}};
        }
    } else if (conformance.provisional) {
        j = json{{"provisionalConform", conformance.condition}};
    } else if (conformance.deprecated) {
        j = json{{"deprecateConform", conformance.condition}};
    } else if (conformance.disallowed) {
        j = json{{"disallowConform", conformance.condition}};
    } else if (!conformance.otherwise.empty()) {
        json otherwise_json;
        for (const auto& otherwise_conformance : conformance.otherwise) {
            if (otherwise_conformance.mandatory) {
                otherwise_json["mandatoryConform"] = otherwise_conformance.condition;
            } else if (otherwise_conformance.optional) {
                otherwise_json["optionalConform"] = otherwise_conformance.condition;
            } else if (otherwise_conformance.provisional) {
                otherwise_json["provisionalConform"] = otherwise_conformance.condition;
            } else if (otherwise_conformance.deprecated) {
                otherwise_json["deprecateConform"] = otherwise_conformance.condition;
            } else if (otherwise_conformance.disallowed) {
                otherwise_json["disallowConform"] = otherwise_conformance.condition;
            }
        }
        j = json{{"otherwiseConform", otherwise_json}};
    } else {
        j = json::object();
    }
}
} // matter

//! This function is used to map Matter other qualities onto an sdfProperty
//! The remaining information gets exported to the sdf-mapping
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::SdfProperty& sdf_property) {
    json quality_json;

    if (other_quality.nullable.has_value()) {
        sdf_property.nullable = other_quality.nullable.value();
    }

    if (other_quality.non_volatile.has_value()) {
        if (other_quality.non_volatile.value()) {
            quality_json["persistence"] = "nonVolatile";
        } else {
            quality_json["persistence"] = "volatile";
        }
    }

    if (other_quality.fixed.has_value()) {
        quality_json["persistence"] = "fixed";
    }

    if (other_quality.scene.has_value()) {
        quality_json["scene"] = other_quality.scene.value();
    }

    if (other_quality.reportable.has_value()) {
        sdf_property.observable = other_quality.reportable.value();
    }

    if (other_quality.change_omitted.has_value()) {
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    }

    if (other_quality.singleton.has_value()) {
        quality_json["singleton"] = other_quality.singleton.value();
    }

    if (other_quality.diagnostics.has_value()) {
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    }

    if (other_quality.large_message.has_value()) {
        quality_json["largeMessage"] = other_quality.large_message.value();
    }

    if (other_quality.quieter_reporting.has_value()) {
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    }

    if (!quality_json.is_null()) {
        current_given_name_node->AddAttribute("quality", quality_json);
    }
}

//! This function is used to map Matter other qualities onto data qualities
//! The remaining information gets exported to the sdf-mapping
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::DataQuality& data_quality) {
    json quality_json;

    if (other_quality.nullable.has_value()) {
        data_quality.nullable = other_quality.nullable.value();
    }

    if (other_quality.non_volatile.has_value()) {
        if (other_quality.non_volatile.value()) {
            quality_json["persistence"] = "nonVolatile";
        } else {
            quality_json["persistence"] = "volatile";
        }
    }

    if (other_quality.fixed.has_value()) {
        quality_json["persistence"] = "fixed";
    }

    if (other_quality.scene.has_value()) {
        quality_json["scene"] = other_quality.scene.value();
    }

    if (other_quality.reportable.has_value()) {
        quality_json["reportable"] = other_quality.reportable.value();
    }

    if (other_quality.change_omitted.has_value()) {
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    }

    if (other_quality.singleton.has_value()) {
        quality_json["singleton"] = other_quality.singleton.value();
    }

    if (other_quality.diagnostics.has_value()) {
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    }

    if (other_quality.large_message.has_value()) {
        quality_json["largeMessage"] = other_quality.large_message.value();
    }

    if (other_quality.quieter_reporting.has_value()) {
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    }

    if (!quality_json.is_null()) {
        current_given_name_node->AddAttribute("quality", quality_json);
    }
}

//! Function used to evaluate the given condition
//! This function is used in combination with Matter conformance's
bool EvaluateConformanceCondition(const json& condition) {
    if (condition.empty()) {
        return true;
    } else if (condition.contains("andTerm")) {
        // Return true, if all the contained expressions evaluate to true
        // Returns false otherwise
        for (auto& item : condition.at("andTerm")) {
            if (!EvaluateConformanceCondition(item)) {
                return false;
            }
        }
        return true;
    } else if (condition.contains("orTerm")) {
        // Returns true, if any one of the contained expressions evaluate to true
        // Returns false otherwise
        for (auto &item: condition.at("orTerm")) {
            if (EvaluateConformanceCondition(item)) {
                return true;
            }
        }
        return false;
    } else if (condition.contains("xorTerm")) {
        // Returns true, if just one of the contained expressions evaluates to true
        // Returns false otherwise
        bool evaluated_one = false;
        for (auto& item : condition.at("xorTerm")) {
            if (EvaluateConformanceCondition(item)) {
                if (!evaluated_one) {
                    evaluated_one = true;
                } else {
                    return true;
                }
            }
            return evaluated_one;
        }
    } else if (condition.contains("notTerm")) {
        return !EvaluateConformanceCondition(condition.at("notTerm"));
    } else if (condition.contains("feature")) {
        if (condition.at("feature").is_array()) {
            for (auto& feature_json : condition.at("feature")) {
                if (supported_features.find(feature_json.at("name")) != supported_features.end()) {
                    return true;
                }
            }
        } else {
            if (supported_features.find(condition.at("feature").at("name")) != supported_features.end()) {
                return true;
            }
        }
    } else if (condition.contains("condition")) {
        // The only condition that this converter can evaluate
        if (condition.at("condition").at("name") == "Matter") {
            return true;
        } else {
            return false;
        }
    } else if (condition.contains("attribute")) {
        std::cout << "Reached" << condition.at("attribute") << std::endl;
        // TODO: Check if the attribute exists
        return true;
    }

    return false;
}

//! Function used to map a Matter bitmap onto a sdfData element
std::pair<std::string, sdf::DataQuality> MapMatterBitmap(const std::pair<std::string, std::list<matter::Bitfield>>& bitmap_pair) {
    auto* bitmap_reference = new ReferenceTreeNode(bitmap_pair.first);
    current_quality_name_node->AddChild(bitmap_reference);
    current_given_name_node = bitmap_reference;

    sdf::DataQuality data_quality;
    json bitmap_json;
    data_quality.type = "array";
    data_quality.unique_items = true;
    sdf::JsoItem item;

    for (const auto& bitfield : bitmap_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        json bitfield_json;
        if (bitfield.conformance.has_value()) {
            bitfield_json.merge_patch(bitfield.conformance.value());
        }

        bitfield_json["bit"] = bitfield.bit;
        bitfield_json["name"] = bitfield.name;
        bitfield_json["summary"] = bitfield.summary;
        item.sdf_choice[bitfield.name] = sdf_choice_data_quality;
        bitmap_json.push_back(bitfield_json);
    }

    data_quality.items = item;
    current_given_name_node->AddAttribute("bitfield", bitmap_json);
    return {bitmap_pair.first, data_quality};
}

//! Function used to map a Matter enum onto a sdfData element
std::pair<std::string, sdf::DataQuality> MapMatterEnum(const std::pair<std::string, std::list<matter::Item>>& enum_pair) {
    auto* enum_reference = new ReferenceTreeNode(enum_pair.first);
    current_quality_name_node->AddChild(enum_reference);
    current_given_name_node = enum_reference;

    sdf::DataQuality data_quality;
    json enum_json;

    for (const auto& item : enum_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        sdf_choice_data_quality.const_ = item.value;
        sdf_choice_data_quality.description = item.summary;

        if (item.conformance.has_value()) {
            json item_json;
            item_json["value"] = item.value;
            item_json.merge_patch(item.conformance.value());
            enum_json.push_back(item_json);
        }

        data_quality.sdf_choice[item.name] = sdf_choice_data_quality;
    }

    current_given_name_node->AddAttribute("item", enum_json);

    return {enum_pair.first, data_quality};
}

//! Function used to map the default type from Matter onto the variable type of sdf
std::optional<sdf::VariableType> MapMatterDefaultType(const matter::DefaultType& default_type) {
    sdf::VariableType variable_type;

    if (std::holds_alternative<double>(default_type)) {
        variable_type = std::get<double>(default_type);
    } else if (std::holds_alternative<int64_t>(default_type)) {
        variable_type = std::get<int64_t>(default_type);
    } else if (std::holds_alternative<uint64_t>(default_type)) {
        variable_type = std::get<uint64_t>(default_type);
    } else if (std::holds_alternative<std::string>(default_type)) {
        if (std::get<std::string>(default_type) == "MS") {
            current_given_name_node->AddAttribute("default", std::get<std::string>(default_type));
            return std::nullopt;
        } else {
            variable_type = std::get<std::string>(default_type);
        }
    } else if (std::holds_alternative<bool>(default_type)) {
        variable_type = std::get<bool>(default_type);
    } else if (std::holds_alternative<std::optional<std::monostate>>(default_type)) {
        variable_type = std::nullopt;
    }

    return variable_type;
}

//! Function used to map a given matter type onto a data quality
void MapMatterType(const std::string& matter_type, sdf::DataQuality& data_quality) {
    // Base Matter data types
    // Boolean data type
    if (matter_type == "bool") {
        data_quality.type = "boolean";
    }
    // Bitmap data type
    else if (matter_type.substr(0, 3) == "map") {
        data_quality.type = "array";
        data_quality.unique_items = true;
        if (matter_type.substr(4) == "8") {
            data_quality.max_items = 8;
        } else if (matter_type.substr(4) == "16") {
            data_quality.max_items = 16;
        } else if (matter_type.substr(4) == "32") {
            data_quality.max_items = 32;
        } else if (matter_type.substr(4) == "64") {
            data_quality.max_items = 64;
        }
    }
    // Unsigned integer data type
    else if (matter_type.substr(0, 4) == "uint") {
        // TODO: These boundaries change if the corresponding value is nullable
        data_quality.type = "integer";
        data_quality.minimum = 0;
        if (matter_type.substr(4) == "8") {
            data_quality.maximum = MATTER_U_INT_8_MAX;
        } else if (matter_type.substr(4) == "16") {
            data_quality.maximum = MATTER_U_INT_16_MAX;
        } else if (matter_type.substr(4) == "24") {
            data_quality.maximum = MATTER_U_INT_24_MAX;
        } else if (matter_type.substr(4) == "32") {
            data_quality.maximum = MATTER_U_INT_32_MAX;
        } else if (matter_type.substr(4) == "40") {
            data_quality.maximum = MATTER_U_INT_40_MAX;
        } else if (matter_type.substr(4) == "48") {
            data_quality.maximum = MATTER_U_INT_48_MAX;
        } else if (matter_type.substr(4) == "56") {
            data_quality.maximum = MATTER_U_INT_56_MAX;
        } else if (matter_type.substr(4) == "64") {
            data_quality.maximum = std::numeric_limits<uint64_t>::max();
        }
    }
    // Signed integer data type
    else if (matter_type.substr(0, 3) == "int") {
        data_quality.type = "integer";
        if (matter_type.substr(3) == "8") {
            data_quality.minimum = MATTER_INT_8_MIN;
            data_quality.maximum = MATTER_INT_8_MAX;
        } else if (matter_type.substr(3) == "16") {
            data_quality.minimum = MATTER_INT_16_MIN;
            data_quality.maximum = MATTER_INT_16_MAX;
        } else if (matter_type.substr(3) == "24") {
            data_quality.minimum = MATTER_INT_24_MIN;
            data_quality.maximum = MATTER_INT_24_MAX;
        } else if (matter_type.substr(3) == "32") {
            data_quality.minimum = MATTER_INT_32_MIN;
            data_quality.maximum = MATTER_INT_32_MAX;
        } else if (matter_type.substr(3) == "40") {
            data_quality.minimum = MATTER_INT_40_MIN;
            data_quality.maximum = MATTER_INT_40_MAX;
        } else if (matter_type.substr(3) == "48") {
            data_quality.minimum = MATTER_INT_48_MIN;
            data_quality.maximum = MATTER_INT_48_MAX;
        } else if (matter_type.substr(3) == "56") {
            data_quality.minimum = MATTER_INT_56_MIN;
            data_quality.maximum = MATTER_INT_56_MAX;
        } else if (matter_type.substr(3) == "64") {
            data_quality.minimum = std::numeric_limits<int64_t>::min();
            data_quality.maximum = std::numeric_limits<int64_t>::max();
        }
    }
    // Single precision floating point data type
    else if (matter_type == "single") {
        data_quality.type = "number";
        data_quality.minimum = std::numeric_limits<float>::min();
        data_quality.minimum = std::numeric_limits<float>::max();
    }
    // Double precision floating point data type
    else if (matter_type == "double") {
        data_quality.type = "number";
        data_quality.minimum = std::numeric_limits<double>::min();
        data_quality.minimum = std::numeric_limits<double>::max();
    }
    // Octet string data type
    else if (matter_type == "octstr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // List data type
    else if (matter_type == "list") {
        data_quality.type = "array";
    }
    // Struct data type
    else if (matter_type == "struct") {
        data_quality.type = "object";
    }
    // Derived Matter data types
    // Percent data type
    // Base: uint8
    else if (matter_type == "percent") {
        data_quality.type = "integer";
        data_quality.unit = "%";
        data_quality.minimum = 0;
        data_quality.maximum = 100;
    }
    // Percent 100th data type
    // Base: uint16
    else if (matter_type == "percent100ths") {
        data_quality.type = "integer";
        data_quality.unit = "%";
        data_quality.minimum = 0;
        data_quality.maximum = 10000;
    }
    // Time of day data type
    // Base: struct
    else if (matter_type == "tod") {
        data_quality.type = "object";
        sdf::SdfData properties;
        sdf::DataQuality hours_qualities;
        hours_qualities.type = "integer";
        hours_qualities.minimum = 0;
        hours_qualities.maximum = 23;
        hours_qualities.nullable = true;
        properties["Hours"] = hours_qualities;
        sdf::DataQuality minutes_qualities;
        minutes_qualities.type = "integer";
        minutes_qualities.minimum = 0;
        minutes_qualities.maximum = 59;
        minutes_qualities.nullable = true;
        properties["Minutes"] = minutes_qualities;
        sdf::DataQuality seconds_qualities;
        seconds_qualities.type = "integer";
        seconds_qualities.minimum = 0;
        seconds_qualities.maximum = 59;
        seconds_qualities.nullable = true;
        properties["Seconds"] = seconds_qualities;
        sdf::DataQuality hundredths_qualities;
        hundredths_qualities.type = "integer";
        hundredths_qualities.minimum = 0;
        hundredths_qualities.maximum = 99;
        hundredths_qualities.nullable = true;
        properties["Hundredths"] = hundredths_qualities;
        data_quality.properties = properties;
    }
    // Data data type
    // Base: struct
    else if (matter_type == "date") {
        data_quality.type = "object";
        sdf::SdfData properties;
        sdf::DataQuality year_qualities;
        year_qualities.type = "integer";
        year_qualities.minimum = 0; // Year 1900
        year_qualities.maximum = MATTER_U_INT_8_MAX; // Year 2155
        year_qualities.nullable = true;
        year_qualities.default_ = std::nullopt;
        properties["Year"] = year_qualities;
        sdf::DataQuality month_qualities;
        month_qualities.type = "integer";
        month_qualities.minimum = 1;
        month_qualities.maximum = 12;
        month_qualities.nullable = true;
        month_qualities.default_ = std::nullopt;
        properties["Month"] = month_qualities;
        sdf::DataQuality day_qualities;
        day_qualities.type = "integer";
        day_qualities.minimum = 1;
        day_qualities.maximum = 31;
        day_qualities.nullable = true;
        day_qualities.default_ = std::nullopt;
        properties["Day"] = day_qualities;
        sdf::DataQuality day_of_week_qualities;
        day_of_week_qualities.type = "integer";
        day_of_week_qualities.minimum = 1;
        day_of_week_qualities.maximum = 7;
        day_of_week_qualities.nullable = true;
        day_of_week_qualities.default_ = std::nullopt;
        properties["DayOfWeek"] = day_of_week_qualities;
        data_quality.properties = properties;
        data_quality.required = {"Year", "Month", "Day", "DayOfWeek"};
    }
    // Epoch time in microseconds data type
    // Base: uint64
    else if (matter_type == "epoch-us") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<u_int64_t>::max();
        data_quality.unit = "us";
    }
    // Epoch time in seconds data type
    // Base: uint32
    else if (matter_type == "epoch-s") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
        data_quality.unit = "s";
    }
    // UTC time data type
    // Base: uint32
    // DEPRECATED
    else if (matter_type == "utc") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
        data_quality.unit = "s";
    }
    // POSIX time in milliseconds
    // Base: uint64
    else if (matter_type == "posix-ms") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<u_int64_t>::max();
        data_quality.unit = "ms";

    }
    // System time in microseconds
    // Base: uint64
    else if (matter_type == "systime-us") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<u_int64_t>::max();
        data_quality.unit = "us";
    }
    // System time in milliseconds
    // Base: uint64
    else if (matter_type == "systime-ms") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<u_int64_t>::max();
        data_quality.unit = "ms";
    }
    // Elapsed time in seconds data type
    // Base: uint32
    else if (matter_type == "elapsed-s"){
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
        data_quality.unit = "s";
    }
    // Temperature data type
    // Base: int16
    else if (matter_type == "temperature") {
        data_quality.type = "integer";
        data_quality.minimum = -27315;
        data_quality.maximum = 32767;
    }
    // Power data type
    // Base: int64
    else if (matter_type == "power-mW") {
        data_quality.type = "integer";
        data_quality.unit = "mW";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Amperage data type
    // Base: int64
    else if (matter_type == "amperage-mA") {
        data_quality.type = "integer";
        data_quality.unit = "mA";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Voltage data type
    // Base: int64
    else if (matter_type == "voltage-mW") {
        data_quality.type = "integer";
        data_quality.unit = "mV";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // Energy data type
    // Base: int64
    else if (matter_type == "energy-mWh") {
        data_quality.type = "integer";
        data_quality.unit = "mWh";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
    // 8-bit enumeration data type
    // Base: uint8
    else if (matter_type == "enum8") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
    }
    // 16-bit enumeration data type
    // Base: uint16
    else if (matter_type == "enum16") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Priority data type
    // Base: enum8
    else if (matter_type == "priority") {
        sdf::DataQuality debug_priority;
        debug_priority.label = "DEBUG";
        debug_priority.const_ = 0;
        debug_priority.description = "Information for engineering debugging/troubleshooting";
        sdf::DataQuality info_priority;
        info_priority.label = "INFO";
        info_priority.const_ = 1;
        info_priority.description = "Information that either drives customer facing features or provides insights into device functions that are used to drive analytics use cases";
        sdf::DataQuality critical_priority;
        critical_priority.label = "CRITICAL";
        critical_priority.const_ = 2;
        critical_priority.description = "Information or notification that impacts safety, a critical function, or ongoing reliable operation of the node or application supported on an endpoint";
        data_quality.sdf_choice["DEBUG"] = debug_priority;
        data_quality.sdf_choice["INFO"] = info_priority;
        data_quality.sdf_choice["CRITICAL"] = critical_priority;
    }
    // Status code data type
    // Base: enum8
    else if (matter_type == "status") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
    }
    // Group id data type
    // Base: uint16
    else if (matter_type == "group-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Endpoint number data type
    // Base: uint16
    else if (matter_type == "endpoint-no") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Vendor id data type
    // Base: uint16
    else if (matter_type == "vendor-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Device type id data type
    // Base: uint32
    else if (matter_type == "devtype-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Fabric id data type
    // Base: uint64
    else if (matter_type == "fabric-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // Fabric index data type
    // Base: uint8
    else if (matter_type == "fabric-idx") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
    }
    // Cluster id data type
    // Base: uint32
    else if (matter_type == "cluster-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Attribute id data type
    // Base: uint32
    else if (matter_type == "attrib-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Field id data type
    // Base: uint32
    else if (matter_type == "field-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Event id data type
    // Base: uint32
    else if (matter_type == "event-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Command id data type
    // Base: uint32
    else if (matter_type == "command-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Action id data type
    // Base: uint8
    else if (matter_type == "action-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
    }
    // Transaction id data type
    // Base: uint32
    else if (matter_type == "trans-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Node id data type
    // Base: uint64
    else if (matter_type == "node-id") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // IEEE address data type
    // Base: uint64
    // DEPRECATED
    else if (matter_type == "EUI64") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // Entry index data type
    // Base: uint16
    else if (matter_type == "entry-idx") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Data version data type
    // Base: uint32
    else if (matter_type == "data-ver") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_32_MAX;
    }
    // Event number data type
    // Base: uint64
    else if (matter_type == "event-no") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = std::numeric_limits<uint64_t>::min();
    }
    // Character string data type
    // Base: octstr
    else if (matter_type == "string") {
        data_quality.type = "string";
    }
    // IP address data type
    // Base: ocstr
    else if (matter_type == "ipadr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // IPv4 address data type
    // Base: octstr
    else if (matter_type == "ipv4adr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
        data_quality.min_length = 8;
        data_quality.max_length = 8;
    }
    // IPv6 address data type
    // Base: octstr
    else if (matter_type == "ipv6adr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
        data_quality.min_length = 32;
        data_quality.max_length = 32;
    }
    // IPv6 prefix data type
    // Base: octstr
    else if (matter_type == "ipv6pre") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    // Hardware address data type
    // Base: octstr
    else if (matter_type == "hwadr") {
        data_quality.type = "string";
        data_quality.min_length = 12;
        data_quality.max_length = 16;
    }
    // Semantic tag data type
    // Base: struct
    else if (matter_type == "semtag") {
        data_quality.type = "object";
        sdf::DataQuality mfg_code;
        mfg_code.type = "integer";
        mfg_code.minimum = 0;
        mfg_code.maximum = MATTER_U_INT_16_MAX;
        mfg_code.nullable = true;
        mfg_code.default_ = std::nullopt;
        sdf::DataQuality namespace_id;
        namespace_id.type = "integer";
        namespace_id.minimum = 0;
        namespace_id.maximum = MATTER_U_INT_16_MAX;
        sdf::DataQuality tag;
        tag.type = "integer";
        tag.minimum = 0;
        tag.maximum = MATTER_U_INT_16_MAX;
        sdf::DataQuality label;
        label.type = "string";
        label.max_length = 64;
        label.nullable = true;
        label.default_ = std::nullopt;
        data_quality.required = {"MfgCode", "NamespaceID", "Tag", "Label"};
    }
    // Namespace data type
    // Base: enum8
    else if (matter_type == "namespace") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Tag data type
    // Base: enum8
    else if (matter_type == "tag") {
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_16_MAX;
    }
    // Otherwise, the type is a custom type defined in the data type section
    else {
        data_quality.sdf_ref = sdf_data_location + matter_type;
    }
}

//! Function used to map a Matter struct onto a sdfData element
std::pair<std::string, sdf::DataQuality> MapMatterStruct(const std::pair<std::string, matter::Struct>& struct_pair) {
    sdf::DataQuality data_quality;
    data_quality.type = "object";

    for (const auto& struct_field : struct_pair.second) {
        sdf::DataQuality struct_field_data_quality;
        struct_field_data_quality.label = struct_field.name;
        struct_field_data_quality.description = struct_field.summary;
        MapMatterType(struct_field.type, struct_field_data_quality);
        data_quality.properties[struct_field.name] = struct_field_data_quality;
        if (struct_field.conformance.has_value()) {
            if (struct_field.conformance.value().mandatory) {
                if (EvaluateConformanceCondition(struct_field.conformance.value().condition)) {
                    data_quality.required.push_back(struct_field.name);
                }
            }
        }
    }

    return {struct_pair.first, data_quality};
}

//! Helper function used to map data qualities onto an JsoItem object
sdf::JsoItem DataQualityToJsoItem(const sdf::DataQuality& data_quality) {
    sdf::JsoItem jso_item;

    jso_item.sdf_ref = data_quality.sdf_ref;
    jso_item.description = data_quality.description;
    jso_item.comment = data_quality.comment;
    jso_item.type = data_quality.type;
    jso_item.sdf_choice = data_quality.sdf_choice;
    jso_item.enum_ = data_quality.enum_;
    jso_item.minimum = data_quality.minimum;
    jso_item.maximum = data_quality.maximum;
    jso_item.min_length = data_quality.max_length;
    jso_item.max_length = data_quality.max_length;
    jso_item.format = data_quality.format;
    jso_item.properties = data_quality.properties;
    jso_item.required = data_quality.required;

    return jso_item;
}

//! Function used to map a Matter constraint onto a data quality
void MapMatterConstraint(const matter::Constraint& constraint, sdf::DataQuality& data_quality) {
    if (constraint.type == "desc") {
        json constraint_json;
        constraint_json["type"] = "desc";
        current_given_name_node->AddAttribute("constraint", constraint_json);
    } else if (constraint.type == "allowed") {
        data_quality.const_ = MapMatterDefaultType(constraint.value.value());
    } else if (constraint.type == "between") {
        data_quality.minimum = constraint.min.value();
        data_quality.maximum = constraint.max.value();
    } else if (constraint.type == "min") {
        data_quality.minimum = constraint.min.value();
    } else if (constraint.type == "max") {
        data_quality.maximum = constraint.max.value();
    } else if (constraint.type == "lengthBetween") {
        if (std::holds_alternative<int64_t>(constraint.min.value())) {
            data_quality.min_length = std::get<int64_t>(constraint.min.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.min.value())) {
            data_quality.min_length = std::get<uint64_t>(constraint.min.value());
        }

        if (std::holds_alternative<int64_t>(constraint.max.value())) {
            data_quality.max_length = std::get<int64_t>(constraint.max.value());
        }
        if (std::holds_alternative<uint64_t>(constraint.max.value())) {
            data_quality.max_length = std::get<uint64_t>(constraint.max.value());
        }
    } else if (constraint.type == "minLength") {
        if (std::holds_alternative<int64_t>(constraint.min.value())) {
            data_quality.min_length = std::get<int64_t>(constraint.min.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.min.value())) {
            data_quality.min_length = std::get<uint64_t>(constraint.min.value());
        }
    } else if (constraint.type == "maxLength") {
        if (std::holds_alternative<int64_t>(constraint.max.value())) {
            data_quality.max_length = std::get<int64_t>(constraint.max.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.max.value())) {
            data_quality.max_length = std::get<uint64_t>(constraint.max.value());
        }
    } else if (constraint.type == "countBetween") {
        if (std::holds_alternative<int64_t>(constraint.min.value())) {
            data_quality.min_items = std::get<int64_t>(constraint.min.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.min.value())) {
            data_quality.min_items = std::get<uint64_t>(constraint.min.value());
        }

        if (std::holds_alternative<int64_t>(constraint.max.value())) {
            data_quality.max_items = std::get<int64_t>(constraint.max.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.max.value())) {
            data_quality.max_items = std::get<uint64_t>(constraint.max.value());
        }
    } else if (constraint.type == "minCount") {
        if (std::holds_alternative<int64_t>(constraint.min.value())) {
            data_quality.min_items = std::get<int64_t>(constraint.min.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.min.value())) {
            data_quality.min_items = std::get<uint64_t>(constraint.min.value());
        }
    } else if (constraint.type == "maxCount") {
        if (std::holds_alternative<int64_t>(constraint.max.value())) {
            data_quality.max_items = std::get<int64_t>(constraint.max.value());
        }

        if (std::holds_alternative<uint64_t>(constraint.max.value())) {
            data_quality.max_items = std::get<uint64_t>(constraint.max.value());
        }
    } else if (constraint.type == "entry") {
        sdf::DataQuality entry_quality;
        MapMatterType(constraint.entry_type, entry_quality);
        if (constraint.entry_constraint != nullptr) {
            MapMatterConstraint(*constraint.entry_constraint, entry_quality);
        }
        data_quality.items = DataQualityToJsoItem(entry_quality);
    }
    // char constraints
}

//! Function used to map a Matter access
//! This structure gets completely exported to the sdf-mapping
void MapMatterAccess(const matter::Access& access) {
    json access_json;
    if (access.read.has_value()) {
        access_json["read"] = access.read.value();
    }

    if (access.write.has_value()) {
        access_json["write"] = access.write.value();
    }

    if (access.fabric_scoped.has_value()) {
        access_json["fabricScoped"] = access.fabric_scoped.value();
    }

    if (access.fabric_sensitive.has_value()) {
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    }

    if (!access.read_privilege.empty()) {
        access_json["readPrivilege"] = access.read_privilege;
    }

    if (!access.write_privilege.empty()) {
        access_json["writePrivilege"] = access.write_privilege;
    }

    if (!access.invoke_privilege.empty()) {
        access_json["invokePrivilege"] = access.invoke_privilege;
    }

    if (access.timed.has_value()) {
        access_json["timed"] = access.timed.value();
    }

    current_given_name_node->AddAttribute("access", access_json);
}

//! Function used to map a Matter access onto a readable and writeable of a sdfProperty
//! The remaining information gets exported to the sdf-mapping
void MapMatterAccess(const matter::Access& access, sdf::SdfProperty& sdf_property) {
    json access_json;
    if (access.read.has_value()) {
        sdf_property.readable = access.read.value();
    }

    if (access.write.has_value()) {
        sdf_property.writable = access.write.value();
    }

    if (access.fabric_scoped.has_value()) {
        access_json["fabricScoped"] = access.fabric_scoped.value();
    }

    if (access.fabric_sensitive.has_value()) {
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    }

    if (!access.read_privilege.empty()) {
        access_json["readPrivilege"] = access.read_privilege;
    }

    if (!access.write_privilege.empty()) {
        access_json["writePrivilege"] = access.write_privilege;
    }

    if (!access.invoke_privilege.empty()) {
        access_json["invokePrivilege"] = access.invoke_privilege;
    }

    if (access.timed.has_value()) {
        access_json["timed"] = access.timed.value();
    }

    current_given_name_node->AddAttribute("access", access_json);
}

//! Function used to map a Matter conformance
//! If the conformance is mandatory, the current element gets added to the list of required elements
//! The function also exports the conformance onto the sdf-mapping
//! This function also returns the result of the evaluated condition
bool MapMatterConformance(const matter::Conformance& conformance) {
    if (conformance.mandatory and EvaluateConformanceCondition(conformance.condition)) {
        sdf_required_list.push_back(current_given_name_node->GeneratePointer());
    }

    if (conformance.mandatory) {
        current_given_name_node->AddAttribute("mandatoryConform", conformance.condition);
    } else if (conformance.optional) {
        if (!conformance.choice.empty()) {
            json choice_conformance_json;
            choice_conformance_json.merge_patch(conformance.condition);
            choice_conformance_json["choice"] = conformance.choice;
            if (conformance.choice_more.has_value()) {
                choice_conformance_json["more"] = conformance.choice_more.value();
            }
            current_given_name_node->AddAttribute("optionalConform", choice_conformance_json);
        }
        current_given_name_node->AddAttribute("optionalConform", conformance.condition);
    } else if (conformance.provisional) {
        current_given_name_node->AddAttribute("provisionalConform", conformance.condition);
    } else if (conformance.deprecated) {
        current_given_name_node->AddAttribute("deprecateConform", conformance.condition);
    } else if (conformance.disallowed) {
        current_given_name_node->AddAttribute("disallowConform", conformance.condition);
    } else if (!conformance.otherwise.empty()) {
        json otherwise_json;
        for (const auto& otherwise : conformance.otherwise) {
            if (otherwise.mandatory) {
                otherwise_json["mandatoryConform"] = otherwise.condition;
            } else if (otherwise.optional) {
                otherwise_json["optionalConform"] = otherwise.condition;
            } else if (otherwise.provisional) {
                otherwise_json["provisionalConform"] = otherwise.condition;
            } else if (otherwise.deprecated) {
                otherwise_json["deprecateConform"] = otherwise.condition;
            } else if (otherwise.disallowed) {
                otherwise_json["disallowConform"] = otherwise.condition;
            }
        }
        current_given_name_node->AddAttribute("otherwiseConform", otherwise_json);
    }

    return EvaluateConformanceCondition(conformance.condition);
}

//! Function used to map a list of Matter data fields onto a data quality
sdf::DataQuality MapMatterDataField(const std::list<matter::DataField>& data_field_list) {
    sdf::DataQuality data_quality;
    //id
    //conformance
    //default
    if (data_field_list.empty()) {}
    else if (data_field_list.size() <= 1) {
        data_quality.label = data_field_list.front().name;
        if (data_field_list.front().access.has_value()) {
            MapMatterAccess(data_field_list.front().access.value());
        }

        data_quality.description = data_field_list.front().summary;
        MapMatterType(data_field_list.front().type, data_quality);
        if (data_field_list.front().default_.has_value()) {
            data_quality.default_ = MapMatterDefaultType(data_field_list.front().default_.value());
        }

        if (data_field_list.front().quality.has_value()) {
            MapOtherQuality(data_field_list.front().quality.value(), data_quality);
        }

        if (data_field_list.front().constraint.has_value()) {
            MapMatterConstraint(data_field_list.front().constraint.value(), data_quality);
        }

        if (data_field_list.front().conformance.has_value()) {
            json conformance_json = data_field_list.front().conformance.value();
            current_given_name_node->AddAttribute("field", conformance_json);
        }
    } else {
        data_quality.type = "object";
        for (const auto& field : data_field_list) {
            sdf::DataQuality data_quality_properties;
            data_quality_properties.label = field.name;
            if (field.access.has_value()) {
                MapMatterAccess(field.access.value());
            }

            data_quality_properties.description = field.summary;
            MapMatterType(field.type, data_quality_properties);
            if (field.default_.has_value()) {
                data_quality_properties.default_ = MapMatterDefaultType(field.default_.value());
            }

            if (field.quality.has_value()) {
                MapOtherQuality(field.quality.value(), data_quality_properties);
            }

            if (field.constraint.has_value()) {
                MapMatterConstraint(field.constraint.value(), data_quality_properties);
            }

            data_quality.properties[field.name] = data_quality_properties;
            if (field.conformance.has_value()) {
                if (field.conformance.value().mandatory and EvaluateConformanceCondition(field.conformance.value().condition)) {
                    data_quality.required.push_back(field.name);
                }
            }
        }
    }
    return data_quality;
}

//! Function used to map a Matter event onto a sdfEvent
sdf::SdfEvent MapMatterEvent(const matter::Event& event) {
    sdf::SdfEvent sdf_event;
    // Append the event node to the tree
    auto* event_reference = new ReferenceTreeNode(event.name);
    current_quality_name_node->AddChild(event_reference);
    current_given_name_node = event_reference;
    // Export the id to the mapping
    event_reference->AddAttribute("id", static_cast<uint64_t>(event.id));

    sdf_event.label = event.name;
    if (event.conformance.has_value()) {
        MapMatterConformance(event.conformance.value());
    }

    if (event.access.has_value()) {
        MapMatterAccess(event.access.value());
    }

    sdf_event.description = event.summary;

    // Export priority to the mapping
    event_reference->AddAttribute("priority", event.priority);
    // Map the datafields onto sdfOutputData
    sdf_event.sdf_output_data = MapMatterDataField(event.data);

    return sdf_event;
}

//! Function used to map a Matter client command onto a sdfAction
//! This list of server commands is used to map the response of the command onto the sdfOutputData
sdf::SdfAction MapMatterCommand(const matter::Command& client_command, const std::unordered_map<std::string,
                                matter::Command>& server_commands) {
    sdf::SdfAction sdf_action;
    // Append the client_command node to the tree
    auto* command_reference = new ReferenceTreeNode(client_command.name);
    current_quality_name_node->AddChild(command_reference);
    current_given_name_node = command_reference;

    // Export the id to the mapping
    command_reference->AddAttribute("id", static_cast<uint64_t>(client_command.id));
    sdf_action.label = client_command.name;
    if (client_command.conformance.has_value()) {
        MapMatterConformance(client_command.conformance.value());
    }

    if (client_command.access.has_value()) {
        MapMatterAccess(client_command.access.value());
    }

    sdf_action.description = client_command.summary;
    // client_command.default_

    if (!client_command.command_fields.empty()) {
        sdf_action.sdf_input_data = MapMatterDataField(client_command.command_fields);
    }

    // If the command does not have a response
    // In this case there is nothing left to be done
    if (client_command.response == "N") {}
    // If the client_command only returns a simple status
    else if (client_command.response == "Y") {
        sdf::DataQuality sdf_output_data;
        sdf_output_data.label = "status";
        sdf_output_data.type = "integer";
        sdf_output_data.minimum = 0;
        sdf_output_data.maximum = MATTER_U_INT_16_MAX;
        sdf_action.sdf_output_data = sdf_output_data;
    }
    // Otherwise, the client client_command has a reference to a server client_command
    else {
        sdf_action.sdf_output_data = MapMatterDataField(server_commands.at(client_command.response).command_fields);
    }

    return sdf_action;
}

//! Function used to map a Matter attribute onto a sdfProperty
sdf::SdfProperty MapMatterAttribute(const matter::Attribute& attribute) {
    sdf::SdfProperty sdf_property;
    // Append the attribute node to the tree
    auto* attribute_reference = new ReferenceTreeNode(attribute.name);
    current_quality_name_node->AddChild(attribute_reference);
    current_given_name_node = attribute_reference;

    // Export the id to the mapping
    attribute_reference->AddAttribute("id", static_cast<uint64_t>(attribute.id));
    sdf_property.label = attribute.name;

    if (attribute.conformance.has_value()) {
        MapMatterConformance(attribute.conformance.value());
    }

    if (attribute.access.has_value()) {
        MapMatterAccess(attribute.access.value(), sdf_property);
    }

    sdf_property.description = attribute.summary;

    // Map the Matter type onto data qualities
    MapMatterType(attribute.type, sdf_property);

    if (attribute.constraint.has_value()) {
        MapMatterConstraint(attribute.constraint.value(), sdf_property);
    }

    if (attribute.quality.has_value()) {
        MapOtherQuality(attribute.quality.value(), sdf_property);
    }

    if (attribute.default_.has_value()) {
        sdf_property.default_ = MapMatterDefaultType(attribute.default_.value());
    }

    return sdf_property;
}

//! Function used to map the Matter feature map
//! This function servers two purposes.
//! Firstly, evaluates for each feature, if it is supported and adds it to the global list of supported features
//! Secondly, it generates a JSON structure and exports this structure to the sdf-mapping
void MapFeatureMap(const std::list<matter::Feature>& feature_map) {
    // Evaluate the features while also exporting them to the mapping
    json feature_map_json;
    for (const auto& feature : feature_map) {
        json feature_json;
        feature_json["bit"] = feature.bit;
        feature_json["code"] = feature.code;
        feature_json["name"] = feature.name;
        feature_json["summary"] = feature.summary;

        if (feature.conformance.has_value()) {
            feature_json.merge_patch(feature.conformance.value());

            bool condition = EvaluateConformanceCondition(feature.conformance.value().condition);
            if (feature.conformance.value().mandatory and condition) {
                supported_features.insert(feature.code);
                std::cout << "Supported Feature: " << feature.code << std::endl;
            }
        }
        feature_map_json["feature"].push_back(feature_json);
    }
    if (!feature_map_json.is_null()) {
        current_given_name_node->AddAttribute("features", feature_map_json);
    }
}

//! Function used to map the cluster classification
//! This structure gets completely exported to the sdf-mapping
void MapClusterClassification(const matter::ClusterClassification& cluster_classification) {
    json cluster_classification_json;

    if (!cluster_classification.hierarchy.empty()) {
        cluster_classification_json["hierarchy"] = cluster_classification.hierarchy;
    }

    if (!cluster_classification.role.empty()) {
        cluster_classification_json["role"] = cluster_classification.role;
    }

    if (!cluster_classification.pics_code.empty()) {
        cluster_classification_json["picsCode"] = cluster_classification.pics_code;
    }

    if (!cluster_classification.scope.empty()) {
        cluster_classification_json["scope"] = cluster_classification.scope;
    }

    if (!cluster_classification.base_cluster.empty()) {
        cluster_classification_json["baseCluster"] = cluster_classification.base_cluster;
    }

    if (!cluster_classification.primary_transaction.empty()) {
        cluster_classification_json["primaryTransaction"] = cluster_classification.primary_transaction;
    }

    current_given_name_node->AddAttribute("classification", cluster_classification_json);
}

//! Function used to map a Matter cluster onto a sdfObject
sdf::SdfObject MapMatterCluster(const matter::Cluster& cluster) {
    sdf::SdfObject sdf_object;
    ReferenceTreeNode* cluster_reference;
    // When combined with a device type definition, we have to differentiate between server and client clusters, as they
    // differentiate in the way they are translated. Furthermore, as a device type can have the same cluster as a server
    // as well as a client cluster at the same time, we have to add an appendix to their given name, as they would
    // otherwise override each other. If side is missing, that indicates, that the cluster is processed without a
    // device type definition.
    if (cluster.side == "client") {
        cluster_reference = new ReferenceTreeNode(cluster.name + "_Client");
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
        cluster_reference->AddAttribute("side", cluster.side);
    } else if (cluster.side == "server"){
        cluster_reference = new ReferenceTreeNode(cluster.name + "_Server");
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
        cluster_reference->AddAttribute("side", cluster.side);
    } else {
        cluster_reference = new ReferenceTreeNode(cluster.name);
        current_quality_name_node->AddChild(cluster_reference);
        current_given_name_node = cluster_reference;
    }
    // Set the location of sdfData
    sdf_data_location = current_given_name_node->GeneratePointer() + "/sdfData/";

    cluster_reference->AddAttribute("id", static_cast<uint64_t>(cluster.id));
    sdf_object.label = cluster.name;
    if (cluster.conformance.has_value()) {
        MapMatterConformance(cluster.conformance.value());
    }

    sdf_object.description = cluster.summary;
    // Export the cluster revision to the mapping
    cluster_reference->AddAttribute("revision", cluster.revision);

    // Export the revision history to the mapping
    json revision_history_json;
    for (const auto& revision : cluster.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json["revision"].push_back(revision_json);
    }
    cluster_reference->AddAttribute("revisionHistory", revision_history_json);

    // Export the cluster aliases to the mapping
    json cluster_aliases_json;
    for (const auto& cluster_alias : cluster.cluster_aliases) {
        json cluster_alias_json;
        cluster_alias_json["id"] = cluster_alias.first;
        cluster_alias_json["name"] = cluster_alias.second;
        cluster_aliases_json["clusterId"].push_back(cluster_alias_json);
    }
    cluster_reference->AddAttribute("clusterIds", cluster_aliases_json);

    if (cluster.classification.has_value()) {
        MapClusterClassification(cluster.classification.value());
    }

    MapFeatureMap(cluster.feature_map);

    // Iterate through the attributes and map them
    auto* sdf_property_node = new ReferenceTreeNode("sdfProperty");
    cluster_reference->AddChild(sdf_property_node);
    current_quality_name_node = sdf_property_node;
    for (const auto& attribute : cluster.attributes){
        sdf::SdfProperty sdf_property = MapMatterAttribute(attribute);
        sdf_object.sdf_property.insert({attribute.name, sdf_property});
    }

    // Iterate through the commands and map them
    auto* sdf_action_node = new ReferenceTreeNode("sdfAction");
    cluster_reference->AddChild(sdf_action_node);
    current_quality_name_node = sdf_action_node;
    for (const auto& command : cluster.client_commands){
        sdf::SdfAction sdf_action = MapMatterCommand(command, cluster.server_commands);
        sdf_object.sdf_action.insert({command.name, sdf_action});
    }

    // Iterate through the events and map them
    auto* sdf_event_node = new ReferenceTreeNode("sdfEvent");
    cluster_reference->AddChild(sdf_event_node);
    current_quality_name_node = sdf_event_node;
    for (const auto& event : cluster.events){
        sdf::SdfEvent sdf_event =  MapMatterEvent(event);
        sdf_object.sdf_event.insert({event.name, sdf_event});
    }

    auto* sdf_data_node = new ReferenceTreeNode("sdfData");
    cluster_reference->AddChild(sdf_data_node);
    current_quality_name_node = sdf_data_node;

    // Iterate through the structs and map them individually
    for (const auto& struct_pair : cluster.structs) {
        sdf_object.sdf_data.insert(MapMatterStruct(struct_pair));
    }

    // Iterate through the enums and map them individually
    for (const auto& enum_pair : cluster.enums) {
        sdf_object.sdf_data.insert(MapMatterEnum(enum_pair));
    }

    // Iterate through bitmaps and map them individually
    for (const auto& bitmap_pair : cluster.bitmaps) {
        sdf_object.sdf_data.insert(MapMatterBitmap(bitmap_pair));
    }

    sdf_object.sdf_required = sdf_required_list;

    return sdf_object;
}

//! Generate a sdf-model or sdf-mapping information block based on information of either
//! a Matter device type or a Matter cluster
sdf::InformationBlock GenerateInformationBlock(const std::variant<matter::Device, matter::Cluster>& input) {
    sdf::InformationBlock information_block;
    if (std::holds_alternative<matter::Device>(input)) {
        information_block.title = std::get<matter::Device>(input).name;
        information_block.description = std::get<matter::Device>(input).summary;
    } else if (std::holds_alternative<matter::Cluster>(input)) {
        information_block.title = std::get<matter::Cluster>(input).name;
        information_block.description = std::get<matter::Cluster>(input).summary;
    }
    return information_block;
}

//! Function used to map the device type classification
//! This structure gets completely exported to the sdf-mapping
void MapDeviceClassification(const matter::DeviceClassification& device_classification) {
    json device_classification_json;

    if (!device_classification.superset.empty()) {
        device_classification_json["superset"] = device_classification.superset;
    }

    if (!device_classification.class_.empty()) {
        device_classification_json["class"] = device_classification.class_;
    }

    if (!device_classification.scope.empty()) {
        device_classification_json["scope"] = device_classification.scope;
    }

    current_given_name_node->AddAttribute("classification", device_classification_json);
}

//! Function used to map a Matter device type onto a sdfThing
sdf::SdfThing MapMatterDevice(const matter::Device& device) {
    sdf::SdfThing sdf_thing;
    // Append a new sdf_object node to the tree
    auto* device_reference = new ReferenceTreeNode(device.name);
    current_quality_name_node->AddChild(device_reference);
    current_given_name_node = device_reference;

    device_reference->AddAttribute("id", static_cast<uint64_t>(device.id));

    if (device.classification.has_value()) {
        MapDeviceClassification(device.classification.value());
    }

    if (device.conformance.has_value()) {
        MapMatterConformance(device.conformance.value());
    }
    // Export the revision history to the mapping
    device_reference->AddAttribute("revision", device.revision);
    json revision_history_json;
    for (const auto& revision : device.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json["revision"].push_back(revision_json);
    }
    device_reference->AddAttribute("revisionHistory", revision_history_json);

    // If the device type contains conditions, export them to the mapping
    if (!device.conditions.empty()) {
        json conditions_json;
        if (device.conditions.size() > 1) {
            for (const auto& condition : device.conditions) {
                conditions_json["condition"].push_back({{"name", condition}});
            }
        } else {
            conditions_json["condition"] = {{"name", device.conditions.front()}};
        }
        device_reference->AddAttribute("conditions", conditions_json);
    }

    sdf_thing.label = device.name;
    sdf_thing.description = device.summary;

    auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
    device_reference->AddChild(sdf_object_reference);
    current_quality_name_node = sdf_object_reference;
    // Iterate through clusters of the device type and map them individually
    for (const auto& cluster : device.clusters){
        sdf::SdfObject sdf_object = MapMatterCluster(cluster);
        // Clear the sdfRequired list as it would result in duplicates
        sdf_object.sdf_required.clear();
        current_quality_name_node = sdf_object_reference;
        // Clear the list of supported features after every run
        supported_features.clear();
        // As a cluster can be mapped as a client as well as a server cluster, we suffix the cluster name
        // with _Client or _Server
        if (cluster.side == "client") {
            sdf_thing.sdf_object.insert({cluster.name + "_Client", sdf_object});
        } else {
            sdf_thing.sdf_object.insert({cluster.name + "_Server", sdf_object});
        }
    }
    // Set the list of required elements for the sdfThing
    sdf_thing.sdf_required = sdf_required_list;

    return sdf_thing;
}

//! Function used to merge device and cluster specifications together
void MergeDeviceCluster(matter::Device& device, const std::list<matter::Cluster>& cluster_list) {
    for (auto& device_cluster : device.clusters) {
        for (const auto& cluster: cluster_list) {
            if (device_cluster.id == cluster.id) {
                matter::Cluster temp_cluster = cluster;
                // Overwrite the conformance for the cluster
                temp_cluster.conformance = device_cluster.conformance;
                // Set the side of the cluster
                temp_cluster.side = device_cluster.side;
                // Overwrite the feature conformance's
                for (auto &device_feature: device_cluster.feature_map) {
                    for (auto &cluster_feature: temp_cluster.feature_map) {
                        if (device_feature.name == cluster_feature.name) {
                            cluster_feature.conformance = device_feature.conformance;
                        }
                    }
                }

                // Overwrite certain attributes
                for (auto &device_attribute: device_cluster.attributes) {
                    for (auto &cluster_attribute: temp_cluster.attributes) {
                        if (device_attribute.name == cluster_attribute.name) {
                            if (device_attribute.access.has_value()) {
                                cluster_attribute.access = device_attribute.access;
                            }
                            if (device_attribute.constraint.has_value()) {
                                cluster_attribute.constraint = device_attribute.constraint;
                            }
                            if (device_attribute.conformance.has_value()) {
                                cluster_attribute.conformance = device_attribute.conformance;
                            }
                            if (device_attribute.quality.has_value()) {
                                cluster_attribute.quality = device_attribute.quality;
                            }
                            if (device_attribute.default_.has_value()) {
                                cluster_attribute.default_ = device_attribute.default_;
                            }
                            if (!device_attribute.type.empty()) {
                                cluster_attribute.type = device_attribute.type;
                            }
                        }
                    }
                }
                // Overwrite certain commands
                for (auto &device_command: device_cluster.server_commands) {
                    for (auto& cluster_client_command : temp_cluster.client_commands) {
                        if (device_command.second.name == cluster_client_command.name) {
                            if (device_command.second.access.has_value()) {
                                cluster_client_command.access = device_command.second.access;
                            }
                            if (device_command.second.conformance.has_value()) {
                                cluster_client_command.conformance = device_command.second.conformance;
                            }
                            if (!device_command.second.response.empty()) {
                                cluster_client_command.response = device_command.second.response;
                            }
                        }
                    }
                    for (auto& cluster_server_command : temp_cluster.server_commands) {
                        if (device_command.second.name == cluster_server_command.second.name) {
                            if (device_command.second.access.has_value()) {
                                cluster_server_command.second.access = device_command.second.access;
                            }
                            if (device_command.second.conformance.has_value()) {
                                cluster_server_command.second.conformance = device_command.second.conformance;
                            }
                            if (!device_command.second.response.empty()) {
                                cluster_server_command.second.response = device_command.second.response;
                            }
                        }
                    }
                }
                // Overwrite certain events
                for (auto &device_event: device_cluster.events) {
                    for (auto &cluster_event: temp_cluster.events) {
                        if (device_event.name == cluster_event.name) {
                            if (device_event.access.has_value()) {
                                cluster_event.access = device_event.access;
                            }
                            if (device_event.conformance.has_value()) {
                                cluster_event.conformance = device_event.conformance;
                            }
                            if (device_event.quality.has_value()) {
                                cluster_event.quality = device_event.quality;
                            }
                            if (!device_event.priority.empty()) {
                                cluster_event.priority = device_event.priority;
                            }
                        }
                    }
                }
                device_cluster = temp_cluster;
            }
        }
    }
}

int MapMatterToSdf(const std::optional<matter::Device>& optional_device, const std::list<matter::Cluster>& cluster_list,
                   sdf::SdfModel& sdf_model, sdf::SdfMapping& sdf_mapping) {
    // Create a new ReferenceTree
    ReferenceTree reference_tree;
    // Check if a device type is given
    if (optional_device.has_value()) {
        // Add sdfThing to the ReferenceTree
        auto* sdf_thing_reference = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(sdf_thing_reference);
        current_quality_name_node = sdf_thing_reference;

        matter::Device device = optional_device.value();
        // Generate the information block based on the given device type
        sdf_model.information_block = GenerateInformationBlock(device);
        sdf_mapping.information_block = GenerateInformationBlock(device);
        // Merge the clusters from the cluster list into the specified clusters for the device type
        MergeDeviceCluster(device, cluster_list);
        // Map the device type onto a sdfThing
        sdf::SdfThing sdf_thing = MapMatterDevice(device);
        sdf_model.sdf_thing.insert({sdf_thing.label, sdf_thing});
    } else {
        // Add sdfObject to the ReferenceTree
        auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(sdf_object_reference);
        current_quality_name_node = sdf_object_reference;
        // Iterate through all clusters and map them individually
        for (const auto& cluster : cluster_list) {
            // Generate the information block based on the given cluster
            sdf_model.information_block = GenerateInformationBlock(cluster);
            sdf_mapping.information_block = GenerateInformationBlock(cluster);
            // Map the cluster onto a sdfObject
            sdf::SdfObject sdf_object = MapMatterCluster(cluster);
            sdf_model.sdf_object.insert({sdf_object.label, sdf_object});
            // Clear the list of required elements
            sdf_required_list.clear();
            current_quality_name_node = sdf_object_reference;
        }
    }

    // Generate the sdf-mapping map section based on the reference tree
    sdf_mapping.map = reference_tree.GenerateMapping(reference_tree.root);

    return 0;
}
