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
std::set<std::string> supported_features;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
std::list<std::string> sdf_required_list;

//! Maps information of the given other quality onto a sdfProperty object
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::SdfProperty& sdf_property)
{
    json quality_json;
    if (other_quality.nullable.has_value())
        sdf_property.nullable = other_quality.nullable.value();
    if (other_quality.non_volatile.has_value())
        quality_json["nonVolatile"] = other_quality.non_volatile.value();
    if (other_quality.fixed.has_value())
        quality_json["fixed"] = other_quality.fixed.value();
    if (other_quality.scene.has_value())
        quality_json["scene"] = other_quality.scene.value();
    if (other_quality.reportable.has_value())
        sdf_property.observable = other_quality.reportable.value();
    if (other_quality.change_omitted.has_value())
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    if (other_quality.singleton.has_value())
        quality_json["singleton"] = other_quality.singleton.value();
    if (other_quality.diagnostics.has_value())
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    if (other_quality.large_message.has_value())
        quality_json["largeMessage"] = other_quality.large_message.value();
    if (other_quality.quieter_reporting.has_value())
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    if (!quality_json.is_null())
        current_given_name_node->AddAttribute("quality", quality_json);
}

//! Maps information of the given other quality onto a data quality object
void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::DataQuality& data_quality)
{
    // TODO: Change nonVolatile to its actual value from the xml
    json quality_json;
    if (other_quality.nullable.has_value())
        data_quality.nullable = other_quality.nullable.value();
    if (other_quality.non_volatile.has_value())
        quality_json["nonVolatile"] = other_quality.non_volatile.value();
    if (other_quality.fixed.has_value())
        quality_json["fixed"] = other_quality.fixed.value();
    if (other_quality.scene.has_value())
        quality_json["scene"] = other_quality.scene.value();
    if (other_quality.reportable.has_value())
        quality_json["reportable"] = other_quality.reportable.value();
    if (other_quality.change_omitted.has_value())
        quality_json["changeOmitted"] = other_quality.change_omitted.value();
    if (other_quality.singleton.has_value())
        quality_json["singleton"] = other_quality.singleton.value();
    if (other_quality.diagnostics.has_value())
        quality_json["diagnostics"] = other_quality.diagnostics.value();
    if (other_quality.large_message.has_value())
        quality_json["largeMessage"] = other_quality.large_message.value();
    if (other_quality.quieter_reporting.has_value())
        quality_json["quieterReporting"] = other_quality.quieter_reporting.value();
    if (!quality_json.is_null())
        current_given_name_node->AddAttribute("quality", quality_json);
}

std::pair<std::string, sdf::DataQuality> MapMatterBitfield(const std::pair<std::string, std::list<matter::Bitfield>>& bitmap_pair)
{
    sdf::DataQuality data_quality;
    data_quality.type = "object";
    for (const auto& bitfield : bitmap_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        sdf_choice_data_quality.const_ = bitfield.bit;
        sdf_choice_data_quality.description = bitfield.summary;
        // conformance
        data_quality.sdf_choice[bitfield.name] = sdf_choice_data_quality;
    }
    //required
    return {bitmap_pair.first, data_quality};
}

std::pair<std::string, sdf::DataQuality> MapMatterEnum(const std::pair<std::string, std::list<matter::Item>>& enum_pair)
{
    sdf::DataQuality data_quality;
    data_quality.type = "object";
    for (const auto& item : enum_pair.second) {
        sdf::DataQuality sdf_choice_data_quality;
        sdf_choice_data_quality.const_ = item.value;
        sdf_choice_data_quality.description = item.summary;
        // conformance
        data_quality.sdf_choice[item.name] = sdf_choice_data_quality;
    }
    //required
    return {enum_pair.first, data_quality};
}

//! Generates data qualities based on the given matter type
void MapMatterType(const std::string& matter_type, sdf::DataQuality& data_quality)
{
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
        }
        else if (matter_type.substr(4) == "16") {
            data_quality.max_items = 16;
        }
        else if (matter_type.substr(4) == "32") {
            data_quality.max_items = 32;
        }
        else if (matter_type.substr(4) == "64") {
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
    }
        // Data data type
        // Base: struct
    else if (matter_type == "date") {
        data_quality.type = "object";
    }
        // Epoch time in microseconds data type
        // Base: uint64
    else if (matter_type == "epoch-us") {}
        // Epoch time in seconds data type
        // Base: uint32
    else if (matter_type == "epoch-s") {}
        // UTC time data type
        // Base: uint32
        // DEPRECATED
    else if (matter_type == "utc") {}
        // POSIX time in milliseconds
        // Base: uint64
    else if (matter_type == "posix-ms") {}
        // System time in microseconds
        // Base: uint64
    else if (matter_type == "systime-us") {}
        // System time in milliseconds
        // Base: uint64
    else if (matter_type == "systime-ms") {}
        // Elapsed time in seconds data type
        // Base: uint32
    else if (matter_type == "elapsed-s") {}
        // Temperature data type
        // Base: int16
    else if (matter_type == "temperature") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.minimum = -27315;
        data_quality.maximum = 32767;
    }
        // Power data type
        // Base: int64
    else if (matter_type == "power-mW") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mW";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
        // Amperage data type
        // Base: int64
    else if (matter_type == "amperage-mA") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mA";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
        // Voltage data type
        // Base: int64
    else if (matter_type == "voltage-mW") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mV";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
        // Energy data type
        // Base: int64
    else if (matter_type == "energy-mWh") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.unit = "mWh";
        data_quality.minimum = std::numeric_limits<int64_t>::min();
        data_quality.maximum = std::numeric_limits<int64_t>::max();
    }
        // 8-bit enumeration data type
        // Base: uint8
    else if (matter_type == "enum8") {}
        // 16-bit enumeration data type
        // Base: uint16
    else if (matter_type == "enum16") {}
        // Priority data type
        // Base: enum8
    else if (matter_type == "priority") {
        data_quality.label = matter_type;
        data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;
        sdf::DataQuality debug_priority;
        //debug_priority.label = "DEBUG";
        debug_priority.const_ = 0;
        debug_priority.description = "Information for engineering debugging/troubleshooting";
        sdf::DataQuality info_priority;
        //info_priority.label = "INFO";
        info_priority.const_ = 1;
        info_priority.description = "Information that either drives customer facing features or provides insights into device functions that are used to drive analytics use cases";
        sdf::DataQuality critical_priority;
        //critical_priority.label = "CRITICAL";
        critical_priority.const_ = 2;
        critical_priority.description = "Information or notification that impacts safety, a critical function, or ongoing reliable operation of the node or application supported on an endpoint";
        data_quality.sdf_choice["DEBUG"] = debug_priority;
        data_quality.sdf_choice["INFO"] = info_priority;
        data_quality.sdf_choice["CRITICAL"] = critical_priority;
    }
        // Status code data type
        // Base: enum8
    else if (matter_type == "status") {
        data_quality.label = matter_type;
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
    else if (matter_type == "action-id") {data_quality.type = "integer";
        data_quality.minimum = 0;
        data_quality.maximum = MATTER_U_INT_8_MAX;}
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
        data_quality.maximum = std::numeric_limits<uint64_t>::min();}
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
        // mfg_code.default_ = null;
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
        //label.default_ = null;
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
        data_quality.label = matter_type;
        //data_quality.sdf_ref
        std::cout << "Found: " << matter_type << std::endl;
    }
}

//! Matter Constraint -> Data Quality
void MapMatterConstraint(const matter::Constraint& constraint, sdf::DataQuality& data_quality)
{
    // We ignore the "desc" constraint type as its dependent on the implementation of the cluster
    if (data_quality.type == "number" or data_quality.type == "integer") {
        if (constraint.value.has_value()) {}
        //data_quality.const_ = constraint.value;
        if (constraint.min.has_value())
            data_quality.minimum = constraint.min.value();
        if (constraint.max.has_value())
            data_quality.maximum = constraint.max.value();
    }
    else if (data_quality.type == "string") {
        if (constraint.value.has_value()) {}
        //data_quality.const_ = constraint.value;
        if (constraint.min.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.min_length = std::get<uint64_t>(constraint.min.value());
        }

        if (constraint.max.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.max.value()))
                data_quality.max_length = std::get<uint64_t>(constraint.max.value());
        }
    }
    else if (data_quality.type == "array") {
        if (constraint.value.has_value()) {}
        //data_quality.const_ = constraint.value;
        if (constraint.min.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.min_items = std::get<uint64_t>(constraint.min.value());
        }
        if (constraint.max.has_value()) {
            if (std::holds_alternative<uint64_t>(constraint.min.value()))
                data_quality.max_items = std::get<uint64_t>(constraint.min.value());
        }
    }
    else if (data_quality.type == "object") {}
}

//! Matter Access Type -> SDF Mapping
//! This function is used standalone to move all qualities to the SDF Mapping
void MapMatterAccess(const matter::Access& access)
{
    json access_json;
    if (access.read.has_value())
        access_json["read"] = access.read.value();
    if (access.write.has_value())
        access_json["write"] = access.write.value();
    if (access.fabric_scoped.has_value())
        access_json["fabricScoped"] = access.fabric_scoped.value();
    if (access.fabric_sensitive.has_value())
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    if (!access.read_privilege.empty())
        access_json["readPrivilege"] = access.read_privilege;
    if (!access.write_privilege.empty())
        access_json["writePrivilege"] = access.write_privilege;
    if (!access.invoke_privilege.empty())
        access_json["invokePrivilege"] = access.invoke_privilege;
    if (access.timed.has_value())
        access_json["timed"] = access.timed.value();

    current_given_name_node->AddAttribute("access", access_json);
}

//! Matter Access Type
//! This function is used in combination with a sdfProperty
void MapMatterAccess(const matter::Access& access, sdf::SdfProperty& sdf_property)
{
    json access_json;
    if (access.read.has_value())
        sdf_property.readable = access.read.value();
    if (access.write.has_value())
        sdf_property.writable = access.write.value();
    if (access.fabric_scoped.has_value())
        access_json["fabricScoped"] = access.fabric_scoped.value();
    if (access.fabric_sensitive.has_value())
        access_json["fabricSensitive"] = access.fabric_sensitive.value();
    if (!access.read_privilege.empty())
        access_json["readPrivilege"] = access.read_privilege;
    if (!access.write_privilege.empty())
        access_json["writePrivilege"] = access.write_privilege;
    if (!access.invoke_privilege.empty())
        access_json["invokePrivilege"] = access.invoke_privilege;
    if (access.timed.has_value())
        access_json["timed"] = access.timed.value();

    current_given_name_node->AddAttribute("access", access_json);
}

bool EvaluateConformanceCondition(const json& condition)
{
    if (condition.empty())
        return true;
    else if (condition.contains("andTerm")) {
        // Return true, if all the contained expressions evaluate to true
        // Returns false otherwise
        for (auto& item : condition.at("andTerm")) {
            if (!EvaluateConformanceCondition(item))
                return false;
        }
        return true;
    }
    else if (condition.contains("orTerm")) {
        // Returns true, if any one of the contained expressions evaluate to true
        // Returns false otherwise
        for (auto& item : condition.at("orTerm")) {
            if (EvaluateConformanceCondition(item))
                return true;
        }
        return false;
    }
    else if (condition.contains("xorTerm")) {
        // Returns true, if just one of the contained expressions evaluates to true
        // Returns false otherwise
        bool evaluated_one = false;
        for (auto& item : condition.at("xorTerm")) {
            if (EvaluateConformanceCondition(item)) {
                if (!evaluated_one)
                    evaluated_one = true;
                else
                    return true;
            }
            return evaluated_one;
        }
    }
    else if (condition.contains("notTerm")) {
        return !EvaluateConformanceCondition(condition.at("notTerm"));
    }
    else if (condition.contains("feature")) {
        std::cout << "Reached" << condition.at("feature") << std::endl;
        if (supported_features.find(condition.at("feature").at("name")) != supported_features.end()) {
            std::cout << "Feature" << condition.at("feature") << "supported" << std::endl;
            return true;
        }
    }
    else if (condition.contains("condition")) {
        std::cout << "Reached" << condition.at("condition") << std::endl;
        // TODO: Check if the condition is satisfied
        return true;
    }
    else if (condition.contains("attribute")) {
        std::cout << "Reached" << condition.at("attribute") << std::endl;
        // TODO: Check if the attribute exists
        return true;
    }
    return false;
}

/**
 * @brief Adds element to sdf_required depending on the conformance.
 *
 * This function adds the current element to sdf_required it is set to mandatory via its conformance.
 *
 * @param conformance The conformance to check.
 * @param current_node The reference tree node of the current element.
 * @return 0 on success, negative on failure.
 */
bool MapMatterConformance(const matter::Conformance& conformance) {
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value() and EvaluateConformanceCondition(conformance.condition)) {
            sdf_required_list.push_back(current_given_name_node->GeneratePointer());
        }
    }

    if (conformance.mandatory.has_value())
        current_given_name_node->AddAttribute("mandatoryConform", conformance.condition);
    else if (conformance.optional.has_value())
        current_given_name_node->AddAttribute("optionalConform", conformance.condition);
    else if (conformance.provisional.has_value())
        current_given_name_node->AddAttribute("provisionalConform", conformance.condition);
    else if (conformance.deprecated.has_value())
        current_given_name_node->AddAttribute("deprecatedConform", conformance.condition);
    else if (conformance.disallowed.has_value())
        current_given_name_node->AddAttribute("disallowConform", conformance.condition);

    return EvaluateConformanceCondition(conformance.condition);
}

sdf::DataQuality MapMatterDataField(const std::list<matter::DataField>& data_field_list)
{
    sdf::DataQuality data_quality;
    //id
    //conformance
    //default
    if (data_field_list.empty()) {}
    else if (data_field_list.size() <= 1) {
        data_quality.label = data_field_list.front().name;
        if (data_field_list.front().access.has_value())
            MapMatterAccess(data_field_list.front().access.value());
        data_quality.description = data_field_list.front().summary;
        MapMatterType(data_field_list.front().type, data_quality);
        if (data_field_list.front().quality.has_value())
            MapOtherQuality(data_field_list.front().quality.value(), data_quality);
        if (data_field_list.front().constraint.has_value())
            MapMatterConstraint(data_field_list.front().constraint.value(), data_quality);
    } else {
        data_quality.type = "object";
        for (const auto& field : data_field_list) {
            sdf::DataQuality data_quality_properties;
            data_quality_properties.label = field.name;
            if (field.access.has_value())
                MapMatterAccess(field.access.value());
            data_quality_properties.description = field.summary;
            MapMatterType(field.type, data_quality_properties);
            if (field.quality.has_value())
                MapOtherQuality(field.quality.value(), data_quality_properties);
            if (field.constraint.has_value())
                MapMatterConstraint(field.constraint.value(), data_quality);
            data_quality.properties[field.name] = data_quality_properties;
        }
        //required
    }
    return data_quality;
}

sdf::SdfEvent MapMatterEvent(const matter::Event& event)
{
    sdf::SdfEvent sdf_event;
    // Append the event node to the tree
    auto* event_reference = new ReferenceTreeNode(event.name);
    current_quality_name_node->AddChild(event_reference);
    current_given_name_node = event_reference;
    // Export the id to the mapping
    event_reference->AddAttribute("id", (uint64_t) event.id);

    sdf_event.label = event.name;
    if (event.conformance.has_value())
        MapMatterConformance(event.conformance.value());
    if (event.access.has_value())
        MapMatterAccess(event.access.value());
    sdf_event.description = event.summary;

    // Export priority to the mapping
    event_reference->AddAttribute("priority", event.priority);
    sdf_event.sdf_output_data = MapMatterDataField(event.data);
    // TODO: Event itself should probably not have these qualities
    //if (event.quality.has_value())
    //    MapOtherQuality(event.quality.value(), sdf_event.sdf_output_data.value());

    return sdf_event;
}

sdf::SdfAction MapMatterCommand(const matter::Command& client_command, const std::map<std::string, matter::Command>& server_commands)
{
    sdf::SdfAction sdf_action;
    // Append the client_command node to the tree
    auto* command_reference = new ReferenceTreeNode(client_command.name);
    current_quality_name_node->AddChild(command_reference);
    current_given_name_node = command_reference;
    // If the command does not have a response
    if (client_command.response == "N") {}
        // If the client_command only returns a simple status
    else if (client_command.response == "Y") {
        sdf::DataQuality sdf_output_data;
        sdf_output_data.type = "integer";
        sdf_output_data.minimum = MATTER_INT_16_MIN;
        sdf_output_data.maximum = MATTER_U_INT_16_MAX;
    }
        // Otherwise, the client client_command has a reference to a server client_command
    else {
        sdf_action.sdf_output_data = MapMatterDataField(server_commands.at(client_command.response).command_fields);
    }

    // Export the id to the mapping
    command_reference->AddAttribute("id", (uint64_t) client_command.id);
    sdf_action.label = client_command.name;
    if (client_command.conformance.has_value())
        MapMatterConformance(client_command.conformance.value());
    if (client_command.access.has_value())
        MapMatterAccess(client_command.access.value());
    sdf_action.description = client_command.summary;
    // client_command.default_
    if (!client_command.command_fields.empty())
        sdf_action.sdf_input_data = MapMatterDataField(client_command.command_fields);

    return sdf_action;
}

sdf::SdfProperty MapMatterAttribute(const matter::Attribute& attribute)
{
    sdf::SdfProperty sdf_property;
    // Append the attribute node to the tree
    auto* attribute_reference = new ReferenceTreeNode(attribute.name);
    current_quality_name_node->AddChild(attribute_reference);
    current_given_name_node = attribute_reference;

    // Export the id to the mapping
    attribute_reference->AddAttribute("id", (uint64_t) attribute.id);
    sdf_property.label = attribute.name;

    if (attribute.conformance.has_value())
        MapMatterConformance(attribute.conformance.value());

    if (attribute.access.has_value())
        MapMatterAccess(attribute.access.value(), sdf_property);

    sdf_property.description = attribute.summary;

    // Map the Matter type onto data qualities
    MapMatterType(attribute.type, sdf_property);

    if (attribute.constraint.has_value())
        MapMatterConstraint(attribute.constraint.value(), sdf_property);

    if (attribute.quality.has_value())
        MapOtherQuality(attribute.quality.value(), sdf_property);

    // sdf_property.default_ = attribute.default_;

    return sdf_property;
}

void MapFeatureMap(const std::list<matter::Feature>& feature_map)
{
    // Evaluate the features while also exporting them to the mapping
    json feature_map_json;
    for (const auto& feature : feature_map) {
        json feature_json;
        bool condition;
        feature_json["bit"] = feature.bit;
        feature_json["code"] = feature.code;
        feature_json["name"] = feature.name;
        feature_json["summary"] = feature.summary;
        if (feature.conformance.has_value()) {
            if (feature.conformance.value().mandatory.has_value())
                feature_json["mandatoryConform"] = feature.conformance.value().condition;
            if (feature.conformance.value().optional.has_value())
                feature_json["optionalConform"] = feature.conformance.value().condition;
            if (feature.conformance.value().provisional.has_value())
                feature_json["provisionalConform"] = feature.conformance.value().condition;
            if (feature.conformance.value().deprecated.has_value())
                feature_json["deprecatedConform"] = feature.conformance.value().condition;
            if (feature.conformance.value().disallowed.has_value())
                feature_json["disallowConform"] = feature.conformance.value().condition;
            condition = EvaluateConformanceCondition(feature.conformance.value().condition);
            if (feature.conformance.value().mandatory.has_value() and condition) {
                supported_features.insert(feature.code);
                std::cout << "Supported Feature: " << feature.code << std::endl;
            }
        }
        feature_map_json.push_back(feature_json);
    }
    if (!feature_map_json.is_null())
        current_given_name_node->AddAttribute("features", feature_map_json);
}

void MapClusterClassification(const matter::ClusterClassification& cluster_classification)
{
    json cluster_classification_json;
    cluster_classification_json["hierarchy"] = cluster_classification.hierarchy;
    cluster_classification_json["role"] = cluster_classification.role;
    cluster_classification_json["picsCode"] = cluster_classification.picsCode;
    cluster_classification_json["scope"] = cluster_classification.scope;
    cluster_classification_json["baseCluster"] = cluster_classification.base_cluster;
    cluster_classification_json["primaryTransaction"] = cluster_classification.primary_transaction;
    current_given_name_node->AddAttribute("classification", cluster_classification_json);
}

sdf::SdfObject MapMatterCluster(const matter::Cluster& cluster)
{
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

    cluster_reference->AddAttribute("id", (uint64_t) cluster.id);
    sdf_object.label = cluster.name;
    if (cluster.conformance.has_value())
        MapMatterConformance(cluster.conformance.value());

    sdf_object.description = cluster.summary;
    cluster_reference->AddAttribute("revision", cluster.revision);
    // Export the revision history to the mapping
    json revision_history_json;
    for (const auto& revision : cluster.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json.push_back(revision_json);
    }
    cluster_reference->AddAttribute("revisionHistory", revision_history_json);

    if (cluster.classification.has_value())
        MapClusterClassification(cluster.classification.value());

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

    for (const auto& enum_pair : cluster.enums) {
        sdf_object.sdf_data.insert(MapMatterEnum(enum_pair));
    }

    for (const auto& bitmap_pair : cluster.bitmaps) {
        sdf_object.sdf_data.insert(MapMatterBitfield(bitmap_pair));
    }

    sdf_object.sdf_required = sdf_required_list;

    return sdf_object;
}

//! Generate a SDF-Model or SDF-Mapping information block based on information of either
//! a Matter device or a Matter cluster
sdf::InformationBlock GenerateInformationBlock(const std::variant<matter::Device, matter::Cluster>& input)
{
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

void MapDeviceClassification(const matter::DeviceClassification& device_classification)
{
    json device_classification_json;
    device_classification_json["superset"] = device_classification.superset;
    device_classification_json["class"] = device_classification.class_;
    device_classification_json["scope"] = device_classification.scope;
    current_given_name_node->AddAttribute("classification", device_classification_json);
}

sdf::SdfThing MapMatterDevice(const matter::Device& device)
{
    sdf::SdfThing sdf_thing;
    // Append a new sdf_object node to the tree
    auto* device_reference = new ReferenceTreeNode(device.name);
    current_quality_name_node->AddChild(device_reference);
    current_given_name_node = device_reference;

    device_reference->AddAttribute("id", (uint64_t) device.id);
    if (device.classification.has_value())
        MapDeviceClassification(device.classification.value());
    if (device.conformance.has_value())
        MapMatterConformance(device.conformance.value());
    // Export the revision history to the mapping
    device_reference->AddAttribute("revision", device.revision);
    json revision_history_json;
    for (const auto& revision : device.revision_history) {
        json revision_json;
        revision_json["revision"] = revision.first;
        revision_json["summary"] = revision.second;
        revision_history_json.push_back(revision_json);
    }
    device_reference->AddAttribute("revisionHistory", revision_history_json);

    sdf_thing.label = device.name;
    sdf_thing.description = device.summary;

    // Iterate through cluster definitions for the device
    auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
    device_reference->AddChild(sdf_object_reference);
    current_quality_name_node = sdf_object_reference;
    for (const auto& cluster : device.clusters){
        sdf::SdfObject sdf_object = MapMatterCluster(cluster);
        // Clear the sdfRequired list as it would result in duplicates
        sdf_object.sdf_required.clear();
        if (cluster.side == "client")
            sdf_thing.sdf_object.insert({cluster.name + "_Client", sdf_object});
        else if (cluster.side == "server")
            sdf_thing.sdf_object.insert({cluster.name + "_Server", sdf_object});
        else {
            std::cout << "Something went wrong" << std::endl;
        }
        current_quality_name_node = sdf_object_reference;
        // Clear the list of supported features after every run
        supported_features.clear();
    }
    sdf_thing.sdf_required = sdf_required_list;

    return sdf_thing;
}

//! Function used to merge device and cluster specifications together
void MergeDeviceCluster(matter::Device& device, const std::list<matter::Cluster>& cluster_list)
{
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

                        }
                    }
                }
                // Overwrite certain commands
                for (auto &client_command: device_cluster.client_commands) {

                }
                // Overwrite certain commands
                for (auto &server_command: device_cluster.server_commands) {

                }
                // Overwrite certain events
                for (auto &device_event: device_cluster.events) {
                    for (auto &cluster_event: temp_cluster.events) {
                        if (device_event.name == cluster_event.name) {

                        }
                    }
                }
                device_cluster = temp_cluster;
            }
        }
    }
}

int MapMatterToSdf(const std::optional<matter::Device>& optional_device,
                   const std::list<matter::Cluster>& cluster_list,
                   sdf::SdfModel& sdf_model, sdf::SdfMapping& sdf_mapping)
{
    ReferenceTree reference_tree;
    if (optional_device.has_value()) {
        auto* sdf_thing_reference = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(sdf_thing_reference);
        current_quality_name_node = sdf_thing_reference;
        matter::Device device = optional_device.value();
        sdf_model.information_block = GenerateInformationBlock(device);
        sdf_mapping.information_block = GenerateInformationBlock(device);
        MergeDeviceCluster(device, cluster_list);
        sdf::SdfThing sdf_thing = MapMatterDevice(device);
        sdf_model.sdf_thing.insert({sdf_thing.label, sdf_thing});
    } else {
        auto* sdf_object_reference = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(sdf_object_reference);
        current_quality_name_node = sdf_object_reference;
        for (const auto& cluster : cluster_list) {
            sdf_model.information_block = GenerateInformationBlock(cluster);
            sdf_mapping.information_block = GenerateInformationBlock(cluster);
            sdf::SdfObject sdf_object = MapMatterCluster(cluster);
            sdf_model.sdf_object.insert({sdf_object.label, sdf_object});
            current_quality_name_node = sdf_object_reference;
        }
    }

    sdf_mapping.map = reference_tree.GenerateMapping(reference_tree.root);

    return 0;
}
