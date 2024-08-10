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

#include <limits>
#include "sdf_to_matter.h"
#include "mapping.h"

//! This is a global pointer to the quality name current node
//! This is designed to point at the top level sdf element like
//! for example the `sdfThing` node, not a specific sdfThing
static ReferenceTreeNode* current_quality_name_node = nullptr;

//! This is a global pointer to the given name current node
//! This is designed to point at the given name of an element
//! for example the `OnOff` node, not a top level sdf element
static ReferenceTreeNode* current_given_name_node = nullptr;

static ReferenceTreeNode* current_cluster_name_node = nullptr;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
static std::list<std::string> sdf_required_list;

//! Map containing enums
//! This map is used when the sdf enum quality gets translated into a Matter enum
std::map<std::string, std::list<matter::Item>> global_enum_map;

//! Map containing structs
//! This map is used when an object type data quality gets translated into a global struct
std::map<std::string, matter::Struct> global_struct_map;

//! Map containing bitmaps
//! This map is used when a bitfield compatible set of data qualities gets translated
std::map<std::string, std::list<matter::Bitfield>> global_bitmap_map;

std::string sdf_data_location;

//! Map containing the elements of the sdf mapping
//! This map is used to resolve the elements outsourced into the map
json reference_map;

//! Function used to check, if the given pointer is part of a sdfRequired element
bool CheckForRequired(const std::string& json_pointer) {
    // Check if the JSON pointer itself is contained in the list
    if (contains(sdf_required_list, json_pointer)) {
        return true;
    }
    // Check if the given name of the structure is part of the mao
    else if (contains(sdf_required_list, GetLastPartAfterSlash(json_pointer))) {
        return true;
    } else {
        return false;
    }
}

//! Generically typed for now as the mapping will later contain different types
//! @brief Imports given key value combination from the SDF Mapping file
//! @param name Name of the target field.
//! @return true if the mapping contained the key, false otherwise
template <typename T> bool ImportFromMapping(const std::string& json_pointer, const std::string& field, T& input) {
    if (reference_map.contains(json_pointer)) {
        if (reference_map.at(json_pointer).contains(field)) {
            reference_map.at(json_pointer).at(field).get_to(input);
            return true;
        }
    }
    return false;
}

//! Function used to map the sdf variable type onto the Matter default type
std::optional<matter::DefaultType> MapSdfDefaultValue(const sdf::VariableType& variable_type) {
    if (std::holds_alternative<uint64_t>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<uint64_t>(variable_type);
        return default_type;
    } else if (std::holds_alternative<int64_t>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<int64_t>(variable_type);
        return default_type;
    } else if (std::holds_alternative<double>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<double>(variable_type);
        return default_type;
    } else if (std::holds_alternative<std::string>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<std::string>(variable_type);
        return default_type;
    } else if (std::holds_alternative<bool>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<bool>(variable_type);
        return default_type;
    } else if (std::holds_alternative<std::list<sdf::ArrayItem>>(variable_type)) {
        // Currently they do not seem to be really compatible with each other
        matter::DefaultType default_type;
        //default_type = to_string(std::get<std::list<sdf::ArrayItem>>(variable_type));
        return default_type;
    } else if (std::holds_alternative<std::optional<std::monostate>>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::nullopt;
        return default_type;
    }

    return std::nullopt;
}

//! Imports the access information for the current object from the mapping
std::optional<matter::Access> ImportAccessFromMapping(const std::string& json_pointer) {
    json access_json;
    ImportFromMapping(json_pointer, "access", access_json);
    if (access_json.is_null()) {
        return std::nullopt;
    }
    matter::Access access;
    if (access_json.contains("read")) {
        access_json.at("read").get_to(access.read);
    }

    if (access_json.contains("write")) {
        access_json.at("write").get_to(access.write);
    }

    if (access_json.contains("fabricScoped")) {
        access_json.at("fabricScoped").get_to(access.fabric_scoped);
    }

    if (access_json.contains("fabricSensitive")) {
        access_json.at("fabricSensitive").get_to(access.fabric_sensitive);
    }

    if (access_json.contains("readPrivilege")) {
        access_json.at("readPrivilege").get_to(access.read_privilege);
    }

    if (access_json.contains("writePrivilege")) {
        access_json.at("writePrivilege").get_to(access.write_privilege);
    }

    if (access_json.contains("invokePrivilege"))
        access_json.at("invokePrivilege").get_to(access.invoke_privilege);
    if (access_json.contains("timed"))
        access_json.at("timed").get_to(access.timed);
    return access;
}

//! Import the other qualities information for the current object for the mapping
std::optional<matter::OtherQuality> ImportOtherQualityFromMapping(const std::string& json_pointer) {
    json other_quality_json;
    ImportFromMapping(json_pointer, "quality", other_quality_json);
    if (other_quality_json.is_null()) {
        return std::nullopt;
    }

    matter::OtherQuality other_quality;
    if (other_quality_json.contains("nullable")) {
        other_quality_json.at("nullable").get_to(other_quality.nullable);
    }

    if (other_quality_json.contains("persistence")) {
        std::string persistence = other_quality_json.at("persistence");
        if (persistence == "fixed") {
            other_quality.fixed = true;
        } else if (persistence == "nonVolatile") {
            other_quality.non_volatile = true;
        } else if (persistence == "volatile") {
            other_quality.non_volatile = false;
        }
    }

    if (other_quality_json.contains("scene")) {
        other_quality_json.at("scene").get_to(other_quality.scene);
    }

    if (other_quality_json.contains("reportable")) {
        other_quality_json.at("reportable").get_to(other_quality.reportable);
    }

    if (other_quality_json.contains("changeOmitted")) {
        other_quality_json.at("changeOmitted").get_to(other_quality.change_omitted);
    }

    if (other_quality_json.contains("singleton")) {
        other_quality_json.at("singleton").get_to(other_quality.singleton);
    }

    if (other_quality_json.contains("diagnostics")) {
        other_quality_json.at("diagnostics").get_to(other_quality.diagnostics);
    }

    if (other_quality_json.contains("largeMessage")) {
        other_quality_json.at("largeMessage").get_to(other_quality.large_message);
    }

    if (other_quality_json.contains("quieterReporting")) {
        other_quality_json.at("quieterReporting").get_to(other_quality.quieter_reporting);
    }

    return other_quality;
}

//! Function used to generate a Matter conformance
//! The conformance is either imported from the mapping or determined via the sdfRequired and required qualities
matter::Conformance GenerateMatterConformance(const std::list<std::string>& sdf_required) {
    matter::Conformance conformance;
    json conformance_json;

    if (ImportFromMapping(current_given_name_node->GeneratePointer(), "mandatoryConform", conformance_json)) {
        conformance.mandatory = true;
        conformance.condition = conformance_json;
    } else if (ImportFromMapping(current_given_name_node->GeneratePointer(), "optionalConform", conformance_json)) {
        conformance.optional = true;
        conformance.condition = conformance_json;
    } else if (ImportFromMapping(current_given_name_node->GeneratePointer(), "provisionalConform", conformance_json)) {
        conformance.provisional = true;
        conformance.condition = conformance_json;
    } else if (ImportFromMapping(current_given_name_node->GeneratePointer(), "deprecateConform", conformance_json)) {
        conformance.deprecated = true;
        conformance.condition = conformance_json;
    } else if (ImportFromMapping(current_given_name_node->GeneratePointer(), "disallowConform", conformance_json)) {
        conformance.disallowed = true;
        conformance.condition = conformance_json;
    } else if (ImportFromMapping(current_given_name_node->GeneratePointer(), "otherwiseConform", conformance_json)) {
        for (const auto& otherwise_json : conformance_json.items()) {
            matter::Conformance otherwise_conformance;
            if (otherwise_json.key() == "mandatoryConform") {
                otherwise_conformance.mandatory = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "optionalConform") {
                otherwise_conformance.optional = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "provisionalConform") {
                otherwise_conformance.provisional = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "deprecateConform") {
                otherwise_conformance.deprecated = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "disallowConform") {
                otherwise_conformance.disallowed = true;
                otherwise_conformance.condition = otherwise_json.value();
            }
            conformance.otherwise.push_back(otherwise_conformance);
        }
    }
    // If no conformance can be imported from the mapping, it will be generated based on the sdfRequired quality
    else {
        // Check if the current given name is part of a sdfRequired definition
        if (CheckForRequired(current_given_name_node->GeneratePointer())) {
            conformance.mandatory = true;
        }
        // If not, check if the given sdfRequired itself is true
        else if (!sdf_required.empty() and sdf_required.front() == "true") {
            conformance.mandatory = true;
        }
        // Otherwise the current element is not mandatory
        else {
            conformance.optional = true;
        }
    }
    return conformance;
}

//! Function used to generate a Matter conformance based on the given conformance JSON
//! The conformance is either imported from the mapping or determined via the sdfRequired and required qualities
matter::Conformance GenerateMatterConformance(const std::list<std::string>& sdf_required, json& conformance_json) {
    matter::Conformance conformance;

    if (conformance_json.contains("mandatoryConform")) {
        conformance.mandatory = true;
        conformance.condition = conformance_json;
    } else if (conformance_json.contains("optionalConform")) {
        conformance.optional = true;
        conformance.condition = conformance_json;
    } else if (conformance_json.contains("provisionalConform")) {
        conformance.provisional = true;
        conformance.condition = conformance_json;
    } else if (conformance_json.contains("deprecateConform")) {
        conformance.deprecated = true;
        conformance.condition = conformance_json;
    } else if (conformance_json.contains("disallowConform")) {
        conformance.disallowed = true;
        conformance.condition = conformance_json;
    } else if (conformance_json.contains("otherwiseConform")) {
        for (const auto& otherwise_json : conformance_json.items()) {
            matter::Conformance otherwise_conformance;
            if (otherwise_json.key() == "mandatoryConform") {
                otherwise_conformance.mandatory = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "optionalConform") {
                otherwise_conformance.optional = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "provisionalConform") {
                otherwise_conformance.provisional = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "deprecateConform") {
                otherwise_conformance.deprecated = true;
                otherwise_conformance.condition = otherwise_json.value();
            } else if (otherwise_json.key() == "disallowConform") {
                otherwise_conformance.disallowed = true;
                otherwise_conformance.condition = otherwise_json.value();
            }
            conformance.otherwise.push_back(otherwise_conformance);
        }
    }
    // If no conformance is defined in the json, it will be generated based on the sdfRequired quality
    else {
        // Check if the current given name is part of a sdfRequired definition
        if (CheckForRequired(current_given_name_node->GeneratePointer())) {
            conformance.mandatory = true;
        }
            // If not, check if the given sdfRequired itself is true
        else if (!sdf_required.empty() and sdf_required.front() == "true") {
            conformance.mandatory = true;
        }
            // Otherwise the current element is not mandatory
        else {
            conformance.optional = true;
        }
    }

    return conformance;
}

//! Function used to check if the variant's integer value is equal to another variant's integer value
bool CheckVariantEquals(const std::variant<double, int64_t, uint64_t>& variant, std::variant<int64_t, uint64_t> value) {
    if (std::holds_alternative<int64_t>(variant)) {
        if (std::holds_alternative<int64_t>(value)) {
            if (std::get<int64_t>(value) == std::get<int64_t>(variant))
                return true;
        }
        else if (std::holds_alternative<u_int64_t>(value)) {
            if (equals(std::get<uint64_t>(value), std::get<int64_t>(variant))) {
                return true;
            }
        }
    }
    else if (std::holds_alternative<uint64_t>(variant)) {
        if (std::holds_alternative<int64_t>(value)) {
            if (equals(std::get<int64_t>(value), std::get<uint64_t>(variant))) {
                return true;
            }
        }
        else if (std::holds_alternative<uint64_t>(value)) {
            if (std::get<uint64_t>(value) == std::get<uint64_t>(variant)) {
                return true;
            }
        }
    }
    return false;
}

//! Function used to check if the variant's integer value is within the given borders
bool CheckVariantBorders(const std::variant<double, int64_t, uint64_t>& variant,
                         std::variant<int64_t, uint64_t> lower_bound,
                         std::variant<int64_t, uint64_t> upper_bound) {
    // The double type is currently ignored as it is not used for the MapIntegerType function
    if (std::holds_alternative<int64_t>(variant)) {
        if (std::holds_alternative<int64_t>(lower_bound)) {
            if (std::holds_alternative<int64_t>(upper_bound)) {
                if (std::get<int64_t>(lower_bound) <= std::get<int64_t>(variant) and
                    std::get<int64_t>(variant) <= std::get<int64_t>(upper_bound)) {
                    return true;
                }
            } else if (std::holds_alternative<uint64_t>(upper_bound)) {
                if (std::get<int64_t>(lower_bound) <= std::get<int64_t>(variant) and
                    compare(std::get<int64_t>(variant),std::get<uint64_t>(upper_bound))) {
                    return true;
                }
            }
        }
        else if (std::holds_alternative<uint64_t>(lower_bound)) {
            if (std::holds_alternative<int64_t>(upper_bound)) {
                if (compare(std::get<uint64_t>(lower_bound), std::get<int64_t>(variant)) and
                std::get<int64_t>(variant) <= std::get<int64_t>(upper_bound)) {
                    return true;
                }
            } else if (std::holds_alternative<uint64_t>(upper_bound)) {
                if (compare(std::get<uint64_t>(lower_bound), std::get<int64_t>(variant)) and
                compare(std::get<int64_t>(variant), std::get<uint64_t>(upper_bound))) {
                    return true;
                }
            }
        }
    }
    else if (std::holds_alternative<uint64_t>(variant)) {
        if (std::holds_alternative<int64_t>(lower_bound)) {
            if (std::holds_alternative<int64_t>(upper_bound)) {
                if (compare(std::get<int64_t>(lower_bound), std::get<uint64_t>(variant)) and
                compare(std::get<uint64_t>(variant), std::get<int64_t>(upper_bound))) {
                    return true;
                }
            } else if (std::holds_alternative<uint64_t>(upper_bound)) {
                if (compare(std::get<int64_t>(lower_bound), std::get<uint64_t>(variant)) and
                std::get<uint64_t>(variant) <= std::get<uint64_t>(upper_bound)) {
                    return true;
                }
            }
        }
        else if (std::holds_alternative<uint64_t>(lower_bound)) {
            if (std::holds_alternative<int64_t>(upper_bound)) {
                if (std::get<uint64_t>(lower_bound) <= std::get<uint64_t>(variant) and
                    compare(std::get<uint64_t>(variant), std::get<int64_t>(upper_bound))) {
                    return true;
                }
            } else if (std::holds_alternative<uint64_t>(upper_bound)) {
                if (std::get<uint64_t>(lower_bound) <= std::get<uint64_t>(variant) and
                    std::get<uint64_t>(variant) <= std::get<uint64_t>(upper_bound)) {
                    return true;
                }
            }
        }
    }
    return false;
}

//! Function used to map a integer type data quality onto a Matter type as well as a Matter constraint
std::string MapIntegerType(const sdf::DataQuality& data_quality, matter::Constraint& constraint) {
    if (data_quality.const_.has_value()) {
        constraint.type = "allowed";
        constraint.value = MapSdfDefaultValue(data_quality.const_.value());
    }
    if (data_quality.minimum.has_value()) {
        // Check if the minimum value is positive or negative
        if (CheckVariantBorders(data_quality.minimum.value(), 0, std::numeric_limits<uint64_t>::max())) {
            if (data_quality.maximum.has_value()) {
                if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_8_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_8_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_8_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint8";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_16_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_16_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_16_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint16";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_24_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_24_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_24_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint24";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_32_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_32_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_32_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint32";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_40_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_40_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_40_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint40";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_48_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_48_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_48_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint48";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_56_MAX)) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_56_MAX)) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_U_INT_56_MAX)) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint56";
                } else if (CheckVariantBorders(data_quality.maximum.value(), 0, std::numeric_limits<uint64_t>::max())) {
                    if (!CheckVariantEquals(data_quality.minimum.value(), 0)) {
                        if (!CheckVariantEquals(data_quality.maximum.value(), std::numeric_limits<uint64_t>::max())) {
                            constraint.type = "between";
                            constraint.min = data_quality.minimum.value();
                            constraint.max = data_quality.maximum.value();
                        } else {
                            constraint.type = "min";
                            constraint.min = data_quality.minimum.value();
                        }
                    } else if (!CheckVariantEquals(data_quality.maximum.value(),
                                                   std::numeric_limits<uint64_t>::max())) {
                        constraint.type = "max";
                        constraint.max = data_quality.maximum.value();
                    }
                    return "uint64";
                }
            }
            // If no maximum value exists
            else {
                // If no maximum value exists, we use the largest possible unsigned value
                return "uint64";
            }
        }
        // If minimum is negative (or larger than an uint64_t)
        else {
            if (data_quality.maximum.has_value()) {
                if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_8_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_8_MIN, MATTER_INT_8_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_8_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_8_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_8_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int8";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_16_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_16_MIN, MATTER_INT_16_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_16_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_16_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_16_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int16";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_24_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_24_MIN, MATTER_INT_24_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_24_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_24_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_24_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int24";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_32_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_32_MIN, MATTER_INT_32_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_32_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_32_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            }
                            else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_32_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int32";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_40_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_40_MIN, MATTER_INT_40_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_40_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_40_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_40_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int40";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_48_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_48_MIN, MATTER_INT_48_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_48_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_48_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_48_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int48";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_56_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_56_MIN, MATTER_INT_56_MAX)) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), MATTER_INT_56_MIN)) {
                            if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_56_MAX)) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(), MATTER_INT_56_MAX)) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int56";
                    }
                } else if (CheckVariantBorders(data_quality.minimum.value(), std::numeric_limits<int64_t>::min(), 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), std::numeric_limits<int64_t>::min(),
                                            std::numeric_limits<int64_t>::max())) {
                        if (!CheckVariantEquals(data_quality.minimum.value(), std::numeric_limits<int64_t>::min())) {
                            if (!CheckVariantEquals(data_quality.maximum.value(),
                                                    std::numeric_limits<int64_t>::max())) {
                                constraint.type = "between";
                                constraint.min = data_quality.minimum.value();
                                constraint.max = data_quality.maximum.value();
                            } else {
                                constraint.type = "min";
                                constraint.min = data_quality.minimum.value();
                            }
                        } else if (!CheckVariantEquals(data_quality.maximum.value(),
                                                       std::numeric_limits<int64_t>::max())) {
                            constraint.type = "max";
                            constraint.max = data_quality.maximum.value();
                        }
                        return "int64";
                    }
                }
            }
            // If maximum does not have a value
            else {
                // If no maximum value exists, we use the largest possible signed value
                return "int64";
            }
        }
    }
    // If a maximum value but no minimum value exists
    // In this case we use the smallest data type that can still contain the maximum value
    else if (data_quality.maximum.has_value()) {
        // Check if the maximum value is positive
        if (CheckVariantBorders(data_quality.maximum.value(), 0, std::numeric_limits<uint64_t>::max())) {
            if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_8_MAX)) {
                return "uint8";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_16_MAX)) {
                return "uint16";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_24_MAX)) {
                return "uint24";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_32_MAX)) {
                return "uint32";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_40_MAX)) {
                return "uint40";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_48_MAX)) {
                return "uint48";
            } else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_INT_56_MAX)) {
                return "uint56";
            } else {
                return "uint64";
            }
        }
        // If the value is negative, we use a signed integer
        else {
            if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_8_MIN, MATTER_INT_8_MAX)) {
                return "int8";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_16_MIN, MATTER_INT_16_MAX)) {
                return "int16";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_24_MIN, MATTER_INT_24_MAX)) {
                return "int24";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_32_MIN, MATTER_INT_32_MAX)) {
                return "int32";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_40_MIN, MATTER_INT_40_MAX)) {
                return "int40";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_48_MIN, MATTER_INT_48_MAX)) {
                return "int48";
            } else if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_56_MIN, MATTER_INT_56_MAX)) {
                return "int56";
            } else {
                return "int64";
            }
        }
    }
    // In case that no minimum and maximum values exists, we default to int64
    // This gives the flexibility to use negative values
    // Also, if the target data quality was supposed to be positive, it would have a minimum of 0
    return "int64";
}

//! Helper function used to map a JsoItem object onto a data quality
sdf::DataQuality MapJsoItemToSdfDataQuality(const sdf::JsoItem& jso_item) {
    sdf::DataQuality data_quality;

    data_quality.sdf_ref = jso_item.sdf_ref;
    data_quality.description = jso_item.description;
    data_quality.comment = jso_item.comment;
    data_quality.type = jso_item.type;
    data_quality.sdf_choice = jso_item.sdf_choice;
    data_quality.enum_ = jso_item.enum_;
    data_quality.minimum = jso_item.minimum;
    data_quality.maximum = jso_item.maximum;
    data_quality.min_length = jso_item.min_length;
    data_quality.max_length = jso_item.max_length;
    data_quality.format = jso_item.format;
    data_quality.properties = jso_item.properties;
    data_quality.required = jso_item.required;

    return data_quality;
}

//! Function used to merge the data qualities of the second argument into the data qualities of the first argument
//! Used in combination with sdfChoice to fill the options with all their information
void MergeDataQualities(sdf::DataQuality& data_quality, const sdf::DataQuality& overwrite_data_quality) {
    if (!overwrite_data_quality.type.empty()) {
        data_quality.type = overwrite_data_quality.type;
    } if (!overwrite_data_quality.sdf_choice.empty()) {
        data_quality.sdf_choice = overwrite_data_quality.sdf_choice;
    } if (!overwrite_data_quality.enum_.empty()) {
        data_quality.enum_ = overwrite_data_quality.enum_;
    } if (overwrite_data_quality.const_.has_value()) {
        data_quality.const_ = overwrite_data_quality.const_;
    } if (overwrite_data_quality.default_.has_value()) {
        data_quality.default_ = overwrite_data_quality.default_;
    } if (overwrite_data_quality.minimum.has_value()) {
        data_quality.minimum = overwrite_data_quality.minimum;
    } if (overwrite_data_quality.maximum.has_value()) {
        data_quality.maximum = overwrite_data_quality.maximum;
    } if (overwrite_data_quality.exclusive_minimum.has_value()) {
        data_quality.exclusive_minimum = overwrite_data_quality.exclusive_minimum;
    } if (overwrite_data_quality.exclusive_maximum.has_value()) {
        data_quality.exclusive_maximum = overwrite_data_quality.exclusive_maximum;
    } if (overwrite_data_quality.multiple_of.has_value()) {
        data_quality.multiple_of = overwrite_data_quality.multiple_of;
    } if (overwrite_data_quality.min_length.has_value()) {
        data_quality.min_length = overwrite_data_quality.min_length;
    } if (overwrite_data_quality.max_length.has_value()) {
        data_quality.max_length = overwrite_data_quality.max_length;
    } if (!overwrite_data_quality.pattern.empty()) {
        data_quality.pattern = overwrite_data_quality.pattern;
    } if (!overwrite_data_quality.format.empty()) {
        data_quality.format = overwrite_data_quality.format;
    } if (overwrite_data_quality.min_items.has_value()) {
        data_quality.min_items = overwrite_data_quality.min_items;
    } if (overwrite_data_quality.max_items.has_value()) {
        data_quality.max_items = overwrite_data_quality.max_items;
    } if (overwrite_data_quality.unique_items.has_value()) {
        data_quality.unique_items = overwrite_data_quality.unique_items;
    } if (overwrite_data_quality.items.has_value()) {
        data_quality.items = overwrite_data_quality.items;
    } if (!overwrite_data_quality.properties.empty()) {
        data_quality.properties = overwrite_data_quality.properties;
    } if (!overwrite_data_quality.required.empty()) {
        data_quality.required = overwrite_data_quality.required;
    } if (!overwrite_data_quality.unit.empty()) {
        data_quality.unit = overwrite_data_quality.unit;
    } if (overwrite_data_quality.nullable.has_value()) {
        data_quality.nullable = overwrite_data_quality.nullable;
    } if (!overwrite_data_quality.sdf_type.empty()) {
        data_quality.sdf_type = overwrite_data_quality.sdf_type;
    } if (!overwrite_data_quality.content_format.empty()) {
        data_quality.content_format = overwrite_data_quality.content_format;
    }
}

//! Function used to generate a Matter enum from the sdf enum data quality
//! This function returns the name of the generated enum as a value
std::string MapSdfEnum(const sdf::DataQuality& data_quality)
{
    std::list<matter::Item> matter_enum;
    int i = 0;
    for (const auto& sdf_item : data_quality.enum_) {
        matter::Item matter_item;
        matter::Conformance conformance;
        conformance.mandatory = true;
        matter_item.value = i;
        matter_item.name = sdf_item;
        matter_item.conformance = conformance;
        matter_enum.push_back(matter_item);
        i++;
    }
    i = 0;
    std::string enum_name = "CustomEnum";
    while (true) {
        if (global_enum_map.count(enum_name + std::to_string(i)) == 0) {
            global_enum_map[enum_name + std::to_string(i)] = matter_enum;
            return enum_name + std::to_string(i);
        }
        i++;
    }
}

std::string MapToMatterEnum(const sdf::DataQuality& data_quality) {
    std::list<matter::Item> matter_enum;
    int i = 0;
    for (const auto& sdf_choice_pair : data_quality.sdf_choice) {
        if (sdf_choice_pair.second.const_.has_value()) {
            if (std::holds_alternative<uint64_t>(sdf_choice_pair.second.const_.value()));
        }
        matter::Item matter_item;
        matter::Conformance conformance;
        conformance.mandatory = true;
        matter_item.conformance = conformance;
        matter_item.value = i;
        matter_item.name = sdf_choice_pair.first;
        matter_enum.push_back(matter_item);
        i++;
    }
    i = 0;
    std::string enum_name = "CustomEnum";
    while (true) {
        if (global_enum_map.count(enum_name + std::to_string(i)) == 0) {
            global_enum_map[enum_name + std::to_string(i)] = matter_enum;
            return enum_name + std::to_string(i);
        }
        i++;
    }
}

//! Function used to check if the given data qualities are compatible with the enum data type
bool CheckEnumCompatible(const sdf::DataQuality& data_quality) {
    if (data_quality.type == "Integer" and !data_quality.sdf_choice.empty()) {
        for (const auto& sdf_choice_pair : data_quality.sdf_choice) {
            if (!sdf_choice_pair.second.const_.has_value()) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::string MapToMatterBitmap(const sdf::DataQuality& data_quality) {
    std::list<matter::Bitfield> bitmap;
    int i = 0;
    json bitfield_json;
    // If one exists, get the pointer to the original element
    if (!data_quality.sdf_ref.empty()) {
        // Import additional information from the mapping
        ImportFromMapping(data_quality.sdf_ref, "bitfield", bitfield_json);
        std::cout << bitfield_json << std::endl;
    }
    for (const auto& sdf_choice : data_quality.items.value().sdf_choice) {
        matter::Bitfield bitfield;
        bitfield.bit = i;
        bitfield.name = sdf_choice.first;
        matter::Conformance conformance;
        conformance.mandatory = true;
        bitfield.conformance = conformance;
        if (!bitfield_json.is_null()) {
            for (const auto& array_field : bitfield_json) {
                std::cout << "ARRAY FIELD" << array_field << std::endl;
                if (array_field.contains("name") and array_field.at("name") == sdf_choice.first) {
                    //bitfield.conformance = GenerateMatterConformance(array_field);
                    if (array_field.contains("bit")) {
                        array_field.at("bit").get_to(bitfield.bit);
                    }
                    if (array_field.contains("summary")) {
                        array_field.at("summary").get_to(bitfield.summary);
                    }
                }
            }

        }

        bitmap.push_back(bitfield);
        i++;
    }
    if (!data_quality.sdf_ref.empty()) {
        std::string bitmap_name = GetLastPartAfterSlash(data_quality.sdf_ref);
        global_bitmap_map[bitmap_name] = bitmap;
        return bitmap_name;
    } else {
        i = 0;
        std::string bitmap_name = "CustomBitmap";
        while (true) {
            if (global_bitmap_map.count(bitmap_name + std::to_string(i)) == 0) {
                global_bitmap_map[bitmap_name + std::to_string(i)] = bitmap;
                return bitmap_name + std::to_string(i);
            }
            i++;
        }
    }
}

//! Function used if the given array can be turned into a bitmap
bool CheckBitmapCompatible(const sdf::DataQuality& data_quality) {
    if (data_quality.items.has_value() and
    data_quality.unique_items.has_value() and
    data_quality.unique_items.value() and
    !data_quality.items.value().sdf_choice.empty()) {
        return true;
    }
    return false;
}

// Function prototype for MapSdfDataType
std::string MapSdfDataType(const sdf::DataQuality& data_quality, matter::Constraint& constraint);

//! Function used to map a object type data quality onto a global Matter struct
//! The function returns the name of the created struct for referencing it
std::string MapSdfObjectType(const sdf::DataQuality& data_quality) {
    int i = 0;
    matter::Struct matter_struct;
    json field_json;
    // If one exists, get the pointer to the original element
    if (!data_quality.sdf_ref.empty()) {
        // Import additional information from the mapping
        ImportFromMapping(data_quality.sdf_ref, "field", field_json);
    }
    if (!data_quality.properties.empty()) {
        for (const auto &data_quality_pair : data_quality.properties) {
            matter::DataField field;
            field.id = i;
            field.name = data_quality_pair.first;
            field.summary = data_quality_pair.second.description;
            matter::Constraint constraint;
            field.type = MapSdfDataType(data_quality_pair.second, constraint);
            field.constraint = constraint;
            // Check if there are information that can be retrieved from the mapping
            if (!field_json.is_null()) {
                for (const auto &data_field_json : field_json) {
                    if (data_field_json["name"] == data_quality_pair.first) {
                        //field.conformance = GenerateMatterConformance(data_field_json);
                        //field.quality = ImportOtherQualityFromMapping();
                        //field.access
                        if (data_field_json.contains("id")) {
                            data_field_json.at("id").get_to(field.id);
                        }
                    }
                }
            }
            if (!field.conformance.has_value()) {
                if (contains(data_quality_pair.second.required, data_quality_pair.first)) {
                    matter::Conformance conformance;
                    conformance.mandatory = true;
                    field.conformance = conformance;
                } else {
                    matter::Conformance conformance;
                    conformance.optional = true;
                    field.conformance = conformance;
                }
            }

            if (data_quality_pair.second.default_.has_value()) {
                field.default_ = MapSdfDefaultValue(data_quality_pair.second.default_.value());
            }
            if (data_quality_pair.second.nullable.has_value()) {
                if (field.quality.has_value()) {
                    field.quality.value().nullable = data_quality_pair.second.nullable;
                } else {
                    matter::OtherQuality other_quality;
                    other_quality.nullable = data_quality_pair.second.nullable;
                    field.quality = other_quality;
                }
            }
            i++;
            matter_struct.push_back(field);
        }
        if (!data_quality.sdf_ref.empty()) {
            std::string struct_name = GetLastPartAfterSlash(data_quality.sdf_ref);
            global_struct_map[struct_name] = matter_struct;
            return struct_name;
        } else {
            i = 0;
            std::string struct_name = "CustomStruct";
            while (true) {
                if (global_struct_map.count(struct_name + std::to_string(i)) == 0) {
                    global_struct_map[struct_name + std::to_string(i)] = matter_struct;
                    return struct_name + std::to_string(i);
                }
                i++;
            }
        }
    } else {
        // If the properties quality is empty, we just return struct
        return "struct";
    }
}

//! Function used to determine a Matter type based on the information of the given data quality
std::string MapSdfDataType(const sdf::DataQuality& data_quality, matter::Constraint& constraint) {
    // Check if the data qualities contain a sdfChoice
    //if (!data_quality.sdf_choice.empty()) {
    //    return MapSdfChoice(data_quality);
    //}

    std::string result;
    //if (!data_quality.sdf_ref.empty()) {
    //    return GetLastPartAfterSlash(data_quality.sdf_ref);
    //}

    if (data_quality.type == "number") {
        if (data_quality.const_.has_value()) {
            constraint.type = "allowed";
            constraint.value = MapSdfDefaultValue(data_quality.const_.value());
        }
        if (data_quality.minimum.has_value()) {
            if (data_quality.maximum.has_value()) {
                constraint.type = "between";
                constraint.min = data_quality.minimum.value();
                constraint.max = data_quality.maximum.value();
            } else {
                constraint.type = "min";
                constraint.min = data_quality.minimum.value();
            }
        } else if (data_quality.maximum.has_value()) {
            constraint.type = "max";
            constraint.max = data_quality.maximum.value();
        }
        result = "double";
    } else if (data_quality.type == "string") {
        if (!data_quality.enum_.empty()) {
            result = MapSdfEnum(data_quality);
        } else {
            if (data_quality.min_length.has_value()) {
                if (data_quality.max_length.has_value()) {
                    constraint.type = "lengthBetween";
                    constraint.min = data_quality.min_length.value();
                    constraint.max = data_quality.max_length.value();
                } else {
                    constraint.type = "minLength";
                    constraint.min = data_quality.min_length.value();
                }
            } else if (data_quality.max_length.has_value()) {
                constraint.type = "maxLength";
                constraint.max = data_quality.max_length.value();
            }
            else if (data_quality.sdf_type == "byte-string") {
                result = "octstr";
            } else if (data_quality.sdf_type == "unix-time") {
                result = "posix-ms";
            } else {
                result = "string";
            }
        }
    } else if (data_quality.type == "boolean") {
        result = "bool";
    } else if (data_quality.type == "integer") {
        if (!data_quality.unit.empty()) {
            // If the data quality has a unit, we try to match it with a compatible Matter type
            if (data_quality.unit == "/100") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), 0) and
                        CheckVariantEquals(data_quality.maximum.value(), 100)) {
                        return "percent";
                    }
                }
            }
            else if (data_quality.unit == "/10000") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), 0) and
                        CheckVariantEquals(data_quality.maximum.value(), 10000)) {
                        return "percent100ths";
                    }
                }
            }
            else if (data_quality.unit == "mW") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), std::numeric_limits<int64_t>::min()) and
                        CheckVariantEquals(data_quality.maximum.value(), std::numeric_limits<int64_t>::max())) {
                        return "power-mW";
                    }
                }
            }
            else if (data_quality.unit == "mA") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), std::numeric_limits<int64_t>::min()) and
                        CheckVariantEquals(data_quality.maximum.value(), std::numeric_limits<int64_t>::max())) {
                        return "amperage-mA";
                    }
                }
            }
            else if (data_quality.unit == "mV") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), std::numeric_limits<int64_t>::min()) and
                        CheckVariantEquals(data_quality.maximum.value(), std::numeric_limits<int64_t>::max())) {
                        return "voltage-mW";
                    }
                }
            }
            else if (data_quality.unit == "mWh") {
                if (data_quality.minimum.has_value() and data_quality.maximum.has_value()) {
                    if (CheckVariantEquals(data_quality.minimum.value(), std::numeric_limits<int64_t>::min()) and
                        CheckVariantEquals(data_quality.maximum.value(), std::numeric_limits<int64_t>::max())) {
                        return "energy-mWh";
                    }
                }
            }
            else if (data_quality.unit == "ms") {
                result = "systime-ms";
            }
        }
        result = MapIntegerType(data_quality, constraint);
    } else if (data_quality.type == "array") {
        if (CheckBitmapCompatible(data_quality)) {
            return MapToMatterBitmap(data_quality);
        }
        // If the data quality has minItems or maxItems, create a fitting constraint
        if (data_quality.min_items.has_value()) {
            if (data_quality.max_items.has_value()) {
                constraint.type = "countBetween";
                constraint.min = data_quality.min_items.value();
                constraint.max = data_quality.max_items.value();
            } else {
                constraint.type = "minCount";
                constraint.min = data_quality.min_items.value();
            }
        } else if (data_quality.max_items.has_value()) {
            constraint.type = "maxCount";
            constraint.max = data_quality.max_items.value();
        }
        // If the data quality has items, generate an entry constraint
        if (data_quality.items.has_value()) {
            auto* entry_constraint = new matter::Constraint();
            constraint.entry_type = MapSdfDataType(MapJsoItemToSdfDataQuality(data_quality.items.value()),
                                                   *entry_constraint);
            constraint.entry_constraint = entry_constraint;
        }
        result = "list";
    } else if (data_quality.type == "object") {
        return MapSdfObjectType(data_quality);
    }

    return result;
}

//! Function used to map sdfInputData or a sdfOutputData onto a Matter data field
matter::DataField MapSdfInputOutputData(const sdf::DataQuality& data_quality) {
    matter::DataField data_field;

    data_field.summary = data_quality.description;
    data_field.name = data_quality.label;
    if (data_quality.nullable.has_value()) {
        matter::OtherQuality quality;
        quality.nullable = data_quality.nullable;
        data_field.quality = quality;
    }
    matter::Constraint constraint;
    data_field.type = MapSdfDataType(data_quality, constraint);
    data_field.constraint = constraint;
    if (data_quality.default_.has_value()) {
        data_field.default_ = MapSdfDefaultValue(data_quality.default_.value());
    }

    return data_field;
}

//! Function used to map a sdfChoice onto a list of exclusive data fields
std::list<matter::DataField> MapSdfChoice(const sdf::DataQuality& data_quality) {
    std::list<matter::DataField> data_field_list;

    for (const auto& sdf_choice_pair : data_quality.sdf_choice) {
        sdf::DataQuality merged_data_quality = data_quality;
        MergeDataQualities(merged_data_quality, sdf_choice_pair.second);
        matter::DataField data_field = MapSdfInputOutputData(merged_data_quality);
        data_field_list.push_back(data_field);
    }

    return data_field_list;
}

//! Function used to map a sdfEvent onto a Matter event
matter::Event MapSdfEvent(const std::pair<std::string, sdf::SdfEvent>& sdf_event_pair) {
    matter::Event event;
    auto* sdf_event_reference = new ReferenceTreeNode(sdf_event_pair.first);
    current_quality_name_node->AddChild(sdf_event_reference);
    current_given_name_node = sdf_event_reference;

    ImportFromMapping(sdf_event_reference->GeneratePointer(), "id", event.id);
    event.name = sdf_event_pair.second.label;
    event.summary = sdf_event_pair.second.description;
    event.conformance = GenerateMatterConformance(sdf_event_pair.second.sdf_required);
    if (sdf_event_pair.second.sdf_output_data.has_value()) {
        // Check if sdfOutputData contains a sdfChoice
        if (!sdf_event_pair.second.sdf_output_data.value().sdf_choice.empty()) {
            event.data = MapSdfChoice(sdf_event_pair.second.sdf_output_data.value());
        }
        // Otherwise check if the type sdfOutputData is object
        // In this case each data quality in properties gets mapped to its own data field
        else if (sdf_event_pair.second.sdf_output_data.value().type == "object") {
            int i = 0;
            for (const auto& data_quality_pair : sdf_event_pair.second.sdf_output_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(data_quality_pair.second);
                field.id = i;
                i++;
                event.data.push_back(field);
            }
        }
        // Otherwise the data quality gets mapped to a single data field
        else {
            matter::DataField field = MapSdfInputOutputData(sdf_event_pair.second.sdf_output_data.value());
            field.id = 0;
            event.data.push_back(field);
        }
    }

    return event;
}

//! Function used to map a sdfAction onto a client and optionally onto a server command
std::pair<matter::Command, std::optional<matter::Command>> MapSdfAction(const std::pair<std::string,
                                                                        sdf::SdfAction>& sdf_action_pair) {
    matter::Command client_command;
    auto* sdf_action_reference = new ReferenceTreeNode(sdf_action_pair.first);
    current_quality_name_node->AddChild(sdf_action_reference);
    current_given_name_node = sdf_action_reference;

    ImportFromMapping(sdf_action_reference->GeneratePointer(), "id", client_command.id);
    client_command.name = sdf_action_pair.second.label;
    client_command.conformance = GenerateMatterConformance(sdf_action_pair.second.sdf_required);
    client_command.access = ImportAccessFromMapping(sdf_action_reference->GeneratePointer());
    client_command.summary = sdf_action_pair.second.description;
    client_command.direction = "commandToServer";
    std::optional<matter::Command> optional_server_command;
    // Check if the sdfOutputData has a value
    if (sdf_action_pair.second.sdf_output_data.has_value()) {
        if (sdf_action_pair.second.sdf_output_data.value().minimum.has_value() and
        sdf_action_pair.second.sdf_output_data.value().maximum.has_value() and
        CheckVariantEquals(sdf_action_pair.second.sdf_output_data.value().minimum.value(), 0) and
            CheckVariantEquals(sdf_action_pair.second.sdf_output_data.value().maximum.value(), MATTER_U_INT_16_MAX)) {
                client_command.response = "Y";
        } else {
            matter::Command server_command;
            ImportFromMapping(sdf_action_reference->GeneratePointer(), "id", server_command.id);
            server_command.name = sdf_action_pair.second.label + "Response";
            server_command.conformance = GenerateMatterConformance(sdf_action_pair.second.sdf_required);
            server_command.summary = sdf_action_pair.second.sdf_output_data.value().description;
            server_command.direction = "responseFromServer";

            client_command.response = server_command.name;

            // Check if sdfOutputData contains a sdfChoice
            if (!sdf_action_pair.second.sdf_output_data.value().sdf_choice.empty()) {
                server_command.command_fields = MapSdfChoice(sdf_action_pair.second.sdf_output_data.value());
            }
            // Otherwise, if object is used as a type, the elements of the object have to be mapped individually
            else if (sdf_action_pair.second.sdf_output_data.value().type == "object") {
                uint32_t id = 0;
                for (const auto &quality_pair : sdf_action_pair.second.sdf_output_data.value().properties) {
                    matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                    field.id = id;
                    // If no label is given, set the quality name
                    if (field.name.empty()) {
                        field.name = quality_pair.first;
                    }
                    if (contains(sdf_action_pair.second.sdf_output_data.value().required, quality_pair.first)) {
                        matter::Conformance conformance;
                        conformance.mandatory = true;
                        field.conformance = conformance;
                    } else {
                        matter::Conformance conformance;
                        conformance.optional = true;
                        field.conformance = conformance;
                    }
                    server_command.command_fields.push_back(field);
                    id++;
                }
            }
            // Otherwise sdfOutputData gets mapped to a single data field
            else  {
                matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_output_data.value());
                json conformance_json;
                if (ImportFromMapping(sdf_action_reference->GeneratePointer(), "field", conformance_json)) {
                    field.conformance = GenerateMatterConformance(
                        sdf_action_pair.second.sdf_output_data.value().sdf_required, conformance_json);
                }
                field.id = 0;
                server_command.command_fields.push_back(field);
            }
            optional_server_command = server_command;
        }
    }
    // Otherwise the client command does not have a response value
    else {
        client_command.response = "N";
    }

    // Check if sdfInputData has a value
    if (sdf_action_pair.second.sdf_input_data.has_value()) {
        // Check if sdfInputData contains a sdfChoice
        if (!sdf_action_pair.second.sdf_input_data.value().sdf_choice.empty()) {
            client_command.command_fields = MapSdfChoice(sdf_action_pair.second.sdf_input_data.value());
        }
        // Otherwise, if object is used as a type, the elements of the object have to be mapped individually
        else if (sdf_action_pair.second.sdf_input_data.value().type == "object") {
            uint32_t id = 0;
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                field.id = id;
                // If no label is given, set the quality name
                if (field.name.empty()) {
                    field.name = quality_pair.first;
                }
                if (contains(sdf_action_pair.second.sdf_input_data.value().required, quality_pair.second.label)) {
                    matter::Conformance conformance;
                    conformance.mandatory = true;
                    field.conformance = conformance;
                } else {
                    matter::Conformance conformance;
                    conformance.optional = true;
                    field.conformance = conformance;
                }

                client_command.command_fields.push_back(field);
                id++;
            }
        } else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_input_data.value());
            json conformance_json;
            if (ImportFromMapping(sdf_action_reference->GeneratePointer(), "field", conformance_json)) {
                field.conformance = GenerateMatterConformance(
                    sdf_action_pair.second.sdf_input_data.value().sdf_required,conformance_json);
            }
            field.id = 0;
            client_command.command_fields.push_back(field);
        }
    }

    return {client_command, optional_server_command};
}

//! Function used to map a sdfProperty onto a Matter attribute
matter::Attribute MapSdfProperty(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair) {
    matter::Attribute attribute;
    auto* sdf_property_reference = new ReferenceTreeNode(sdf_property_pair.first);
    current_quality_name_node->AddChild(sdf_property_reference);
    current_given_name_node = sdf_property_reference;

    ImportFromMapping(sdf_property_reference->GeneratePointer(), "id", attribute.id);
    attribute.name = sdf_property_pair.first;
    attribute.conformance = GenerateMatterConformance(sdf_property_pair.second.sdf_required);

    attribute.access = ImportAccessFromMapping(sdf_property_reference->GeneratePointer());
    if (attribute.access.has_value()) {
        attribute.access.value().read = sdf_property_pair.second.readable;
        attribute.access.value().write = sdf_property_pair.second.writable;
    } else {
        if (sdf_property_pair.second.readable.has_value() or sdf_property_pair.second.writable.has_value()) {
            matter::Access access;
            access.read = sdf_property_pair.second.readable;
            access.write = sdf_property_pair.second.writable;
            attribute.access = access;
        }
    }

    attribute.quality = ImportOtherQualityFromMapping(sdf_property_reference->GeneratePointer());
    if (attribute.quality.has_value()) {
        attribute.quality.value().nullable = sdf_property_pair.second.nullable;
        attribute.quality.value().reportable = sdf_property_pair.second.observable;
    } else {
        if (sdf_property_pair.second.observable.has_value() or sdf_property_pair.second.nullable.has_value()) {
            matter::OtherQuality quality;
            quality.nullable = sdf_property_pair.second.nullable;
            quality.reportable = sdf_property_pair.second.observable;
            attribute.quality = quality;
        }
    }

    attribute.summary = sdf_property_pair.second.description;
    matter::Constraint constraint;
    attribute.type = MapSdfDataType(sdf_property_pair.second, constraint);
    json desc_json;
    if (ImportFromMapping(current_given_name_node->GeneratePointer(), "constraint", desc_json)) {
        if (desc_json.contains("type")) {
            desc_json.at("type").get_to(constraint.type);
        }
    }

    attribute.constraint = constraint;
    if (sdf_property_pair.second.default_.has_value()) {
        attribute.default_ = MapSdfDefaultValue(sdf_property_pair.second.default_.value());
    } else {
        ImportFromMapping(current_given_name_node->GeneratePointer(), "default", attribute.default_);
    }

    return attribute;
}

//! Function used to map a sdfChoice onto multiple attributes
//! Each of the resulting attributes gets a choice conformance
std::list<matter::Attribute> MapSdfChoice(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair) {
    std::list<matter::Attribute> attribute_list;

    for (const auto& sdf_choice_pair : sdf_property_pair.second.sdf_choice) {
        sdf::SdfProperty merged_sdf_property = sdf_property_pair.second;
        // Merge the data qualities of the sdfChoice into the sdfProperty
        MergeDataQualities(merged_sdf_property, sdf_choice_pair.second);
        // Map the resulting sdfProperty onto an attribute
        matter::Attribute attribute = MapSdfProperty({sdf_property_pair.first, merged_sdf_property});
        // Create the choice conformance
        matter::Conformance choice_conformance;
        choice_conformance.optional = true;
        choice_conformance.choice = "a";
        attribute.conformance = choice_conformance;
        attribute_list.push_back(attribute);
    }

    return attribute_list;
}

//! Function used to generate a feature map based on the information given by the sdf-mapping
std::list<matter::Feature> GenerateFeatureMap() {
    std::list<matter::Feature> feature_map;
    json feature_map_json;
    if (!ImportFromMapping(current_given_name_node->GeneratePointer(), "features", feature_map_json)) {
        return feature_map;
    }
    for (const auto& feature_json : feature_map_json.at("feature")) {
        matter::Feature feature;
        if (feature_json.contains("bit")) {
            feature_json.at("bit").get_to(feature.bit);
        }

        if (feature_json.contains("code")) {
            feature_json.at("code").get_to(feature.code);
        }

        if (feature_json.contains("name")) {
            feature_json.at("name").get_to(feature.name);
        }

        if (feature_json.contains("summary")) {
            feature_json.at("summary").get_to(feature.summary);
        }

        if (feature_json.contains("mandatoryConform")) {
            matter::Conformance conformance;
            conformance.mandatory = true;
            feature_json.at("mandatoryConform").get_to(conformance.condition);
            feature.conformance = conformance;
        }

        if (feature_json.contains("optionalConform")) {
            matter::Conformance conformance;
            conformance.optional = true;
            feature_json.at("optionalConform").get_to(conformance.condition);
            feature.conformance = conformance;
        }

        if (feature_json.contains("provisionalConform")) {
            matter::Conformance conformance;
            conformance.provisional = true;
            feature.conformance = conformance;
        }

        if (feature_json.contains("deprecateConform")) {
            matter::Conformance conformance;
            conformance.deprecated = true;
            feature.conformance = conformance;
        }

        if (feature_json.contains("disallowConform")) {
            matter::Conformance conformance;
            conformance.disallowed = true;
            feature.conformance = conformance;
        }

        feature_map.push_back(feature);
    }
    return feature_map;
}

//! Function used to generate a cluster classification based on the information given by the sdf-mapping
matter::ClusterClassification GenerateClusterClassification() {
    matter::ClusterClassification cluster_classification;
    json cluster_classification_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "classification", cluster_classification_json);
    if (cluster_classification_json.contains("hierarchy")) {
        cluster_classification_json.at("hierarchy").get_to(cluster_classification.hierarchy);
    }

    if (cluster_classification_json.contains("role")) {
        cluster_classification_json.at("role").get_to(cluster_classification.role);
    }

    if (cluster_classification_json.contains("picsCode")) {
        cluster_classification_json.at("picsCode").get_to(cluster_classification.pics_code);
    }

    if (cluster_classification_json.contains("scope")) {
        cluster_classification_json.at("scope").get_to(cluster_classification.scope);
    }

    if (cluster_classification_json.contains("baseCluster")) {
        cluster_classification_json.at("baseCluster").get_to(cluster_classification.base_cluster);
    }

    if (cluster_classification_json.contains("primaryTransaction")) {
        cluster_classification_json.at("primaryTransaction").get_to(cluster_classification.primary_transaction);
    }

    return cluster_classification;
}

//! Function used to map a sdfObject onto a Matter cluster
matter::Cluster MapSdfObject(const std::pair<std::string, sdf::SdfObject>& sdf_object_pair) {
    matter::Cluster cluster;
    auto* sdf_object_reference = new ReferenceTreeNode(sdf_object_pair.first);
    current_quality_name_node->AddChild(sdf_object_reference);
    current_given_name_node = sdf_object_reference;

    ImportFromMapping(sdf_object_reference->GeneratePointer(), "id", cluster.id);

    if (sdf_object_pair.second.label.empty()) {
        cluster.name = sdf_object_pair.first;
    } else {
        cluster.name = sdf_object_pair.second.label;
    }

    if (!sdf_object_pair.second.sdf_required.empty()) {
        sdf_required_list.insert(sdf_required_list.end(),
                                 sdf_object_pair.second.sdf_required.begin(),
                                 sdf_object_pair.second.sdf_required.end());
    }
    cluster.conformance = GenerateMatterConformance(sdf_object_pair.second.sdf_required);
    cluster.summary = sdf_object_pair.second.description;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "side", cluster.side);
    if (!ImportFromMapping(sdf_object_reference->GeneratePointer(), "revision", cluster.revision)) {
        cluster.revision = 1;
    }

    // Import the revision history from the mapping
    json revision_history_json;
    if (ImportFromMapping(sdf_object_reference->GeneratePointer(), "revisionHistory", revision_history_json)) {
        for (const auto& item : revision_history_json.at("revision")) {
            u_int8_t revision;
            item.at("revision").get_to(revision);
            std::string summary;
            item.at("summary").get_to(summary);
            cluster.revision_history[revision] = summary;
        }
    }

    // Import the cluster aliases from the mapping
    json cluster_aliases_json;
    if (ImportFromMapping(sdf_object_reference->GeneratePointer(), "clusterIds", cluster_aliases_json)) {
        for (const auto& cluster_alias : cluster_aliases_json.at("clusterId")) {
            uint32_t id;
            cluster_alias.at("id").get_to(id);
            std::string name;
            cluster_alias.at("name").get_to(name);
            cluster.cluster_aliases.emplace_back(id, name);
        }
    }

    cluster.classification = GenerateClusterClassification();

    cluster.feature_map = GenerateFeatureMap();

    // Iterate through all sdfProperties and map them individually
    auto* sdf_property_reference = new ReferenceTreeNode("sdfProperty");
    sdf_object_reference->AddChild(sdf_property_reference);
    current_quality_name_node = sdf_property_reference;
    for (const auto& sdf_property_pair : sdf_object_pair.second.sdf_property) {
        // Check if the sdfProperty contains a sdfChoice
        // If yes, multiple exclusive attributes will be generated
        if (!sdf_property_pair.second.sdf_choice.empty()) {
            std::list<matter::Attribute> mapped_properties = MapSdfChoice(sdf_property_pair);
            cluster.attributes.insert(cluster.attributes.end(), mapped_properties.begin(), mapped_properties.end());
        } else {
            cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
        }
    }

    // Iterate through all sdfActions and map them individually
    auto* sdf_action_reference = new ReferenceTreeNode("sdfAction");
    sdf_object_reference->AddChild(sdf_action_reference);
    current_quality_name_node = sdf_action_reference;
    for (const auto& sdf_action_pair : sdf_object_pair.second.sdf_action) {
        std::pair<matter::Command, std::optional<matter::Command>> command_pair = MapSdfAction(sdf_action_pair);
        cluster.client_commands.push_back(command_pair.first);
        if (command_pair.second.has_value()) {
            cluster.server_commands[command_pair.second.value().name] = command_pair.second.value();
        }
    }

    // Iterate through all sdfEvents and map them individually
    auto* sdf_event_reference = new ReferenceTreeNode("sdfEvent");
    sdf_object_reference->AddChild(sdf_event_reference);
    current_quality_name_node = sdf_event_reference;
    for (const auto& sdf_event_pair : sdf_object_pair.second.sdf_event) {
        cluster.events.push_back(MapSdfEvent(sdf_event_pair));
    }

    // Iterate through all sdfData elements and map them individually
    auto* sdf_data_reference = new ReferenceTreeNode("sdfData");
    sdf_object_reference->AddChild(sdf_data_reference);
    current_quality_name_node = sdf_data_reference;

    // If enums have been added to the global map of enums, we merge them into the rest of the enums
    if (!global_enum_map.empty()) {
        cluster.enums.insert(global_enum_map.begin(), global_enum_map.end());
        global_enum_map.clear();
    }

    if (!global_bitmap_map.empty()) {
        cluster.bitmaps.insert(global_bitmap_map.begin(), global_bitmap_map.end());
        global_bitmap_map.clear();
    }

    // If structs have been added to the global map of structs, we merge them into the rest of the structs
    if (!global_struct_map.empty()) {
        cluster.structs.insert(global_struct_map.begin(), global_struct_map.end());
        global_struct_map.clear();
    }

    for (const auto& sdf_data_elem : sdf_object_pair.second.sdf_data) {
        auto* given_sdf_data_reference = new ReferenceTreeNode(sdf_data_elem.first);
        sdf_data_reference->AddChild(given_sdf_data_reference);
        current_given_name_node = given_sdf_data_reference;
        /*
        if (sdf_data_elem.second.type == "object") {
            matter::Struct matter_struct;
            uint32_t id = 0;
            for (const auto& property : sdf_data_elem.second.properties) {
                matter::DataField data_field;
                data_field.id = id;
                data_field.name = property.second.label;
                data_field.summary = property.second.description;
                json conformance_json;
                if (ImportFromMapping(current_given_name_node->GeneratePointer(), "properties", conformance_json)) {
                    if (conformance_json.contains(property.first)) {
                        data_field.conformance = GenerateMatterConformance(property.second.sdf_required,
                                                                           conformance_json.at(property.first));
                    }
                } else {
                    matter::Conformance conformance;
                    if (contains(sdf_data_elem.second.required, property.first)) {
                        conformance.mandatory = true;
                    } else {
                        conformance.optional = true;
                    }
                    data_field.conformance = conformance;
                }
                matter::Constraint constraint;
                data_field.type = MapSdfDataType(property.second, constraint);
                data_field.constraint = constraint;
                matter_struct.push_back(data_field);
                id++;
            }
            cluster.structs[sdf_data_elem.first] = matter_struct;
        } else if (sdf_data_elem.second.type == "array") {
            if (sdf_data_elem.second.items.has_value()) {
                std::list<matter::Bitfield> bitmap;
                int bit = 0;
                for (const auto& sdf_choice : sdf_data_elem.second.items.value().sdf_choice) {
                    matter::Bitfield bitfield;
                    bitfield.name = sdf_choice.first;
                    json conformance_json;
                    if (ImportFromMapping(current_given_name_node->GeneratePointer(), "bitfield", conformance_json)) {
                        for (auto& bitfield_json : conformance_json) {
                            if (bitfield_json.at("name") == bitfield.name) {
                                if (bitfield_json.contains("summary")) {
                                    bitfield_json.at("summary").get_to(bitfield.summary);
                                }
                                if (bitfield_json.contains("bit")) {
                                    bitfield_json.at("bit").get_to(bitfield.bit);
                                } else {
                                    bitfield.bit = bit;
                                    bit++;
                                }
                                bitfield.conformance = GenerateMatterConformance(sdf_choice.second.sdf_required,
                                                                                 bitfield_json);
                            }
                        }
                    }
                    bitmap.push_back(bitfield);
                }
                cluster.bitmaps[sdf_data_elem.first] = bitmap;
            }
        } else if (!sdf_data_elem.second.sdf_choice.empty()) {

            }
            cluster.enums[sdf_data_elem.first] = matter_enum;
        }*/
    }

    return cluster;
}

//! Function used to generate a device type classification based on the information given by sdf-mapping
matter::DeviceClassification GenerateDeviceClassification()
{
    matter::DeviceClassification device_classification;

    json device_classification_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "classification", device_classification_json);
    if (device_classification_json.contains("superset")) {
        device_classification_json.at("superset").get_to(device_classification.superset);
    }

    if (device_classification_json.contains("class")) {
        device_classification_json.at("class").get_to(device_classification.class_);
    }

    if (device_classification_json.contains("scope")) {
        device_classification_json.at("scope").get_to(device_classification.scope);
    }

    return device_classification;
}

//! Function used to map a sdfThing onto a Matter device type
matter::Device MapSdfThing(const std::pair<std::string, sdf::SdfThing>& sdf_thing_pair)
{
    matter::Device device;
    // Add the current sdf_thing to the reference tree
    auto* sdf_thing_reference = new ReferenceTreeNode(sdf_thing_pair.first);
    current_quality_name_node->AddChild(sdf_thing_reference);
    current_given_name_node = sdf_thing_reference;
    // Import the ID from the mapping
    ImportFromMapping(sdf_thing_reference->GeneratePointer(), "id", device.id);
    device.name = sdf_thing_pair.second.label;
    if (!sdf_thing_pair.second.sdf_required.empty()) {
        sdf_required_list.insert(sdf_required_list.end(),
                                 sdf_thing_pair.second.sdf_required.begin(),
                                 sdf_thing_pair.second.sdf_required.end());
    }
    device.summary = sdf_thing_pair.second.description;
    // Import the revision as well as the revision history from the mapping
    if (!ImportFromMapping(sdf_thing_reference->GeneratePointer(), "revision", device.revision)) {
        device.revision = 1;
    }
    json revision_history_json;
    if (ImportFromMapping(sdf_thing_reference->GeneratePointer(), "revisionHistory", revision_history_json)) {
        for (const auto& item : revision_history_json.at("revision")) {
            u_int8_t revision;
            item.at("revision").get_to(revision);
            std::string summary;
            item.at("summary").get_to(summary);
            device.revision_history[revision] = summary;
        }
    }

    // Import the conditions from the mapping
    json conditions_json;
    if (ImportFromMapping(sdf_thing_reference->GeneratePointer(), "conditions", conditions_json)) {
        for (const auto& condition : conditions_json.at("condition")) {
            device.conditions.push_back(condition.at("name"));
        }
    }

    device.classification = GenerateDeviceClassification();

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdf_object_pair : sdf_thing_pair.second.sdf_object) {
        current_quality_name_node = new ReferenceTreeNode("sdfObject");
        sdf_thing_reference->AddChild(current_quality_name_node);
        device.clusters.push_back(MapSdfObject(sdf_object_pair));
    }

    // Check if the sdfThing contains sdfProperties, sdfActions or sdfEvents
    // If so, we create a new Cluster for these elements and add it to the sdfThing
    if (!sdf_thing_pair.second.sdf_property.empty() or !sdf_thing_pair.second.sdf_action.empty() or
        !sdf_thing_pair.second.sdf_event.empty()) {
        matter::Cluster cluster;
        cluster.name = sdf_thing_pair.first;
        // First possible custom Cluster id
        cluster.id = 32768;
        // Iterate through all sdfProperties and map them individually
        for (const auto& sdf_property_pair : sdf_thing_pair.second.sdf_property) {
            current_quality_name_node = new ReferenceTreeNode("sdfProperty");
            sdf_thing_reference->AddChild(current_quality_name_node);
            cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
        }
        // Iterate through all sdfActions and map them individually
        for (const auto& sdf_action_pair : sdf_thing_pair.second.sdf_action) {
            current_quality_name_node = new ReferenceTreeNode("sdfAction");
            sdf_thing_reference->AddChild(current_quality_name_node);
            std::pair<matter::Command, std::optional<matter::Command>> command_pair = MapSdfAction(sdf_action_pair);
            cluster.client_commands.push_back(command_pair.first);
            if (command_pair.second.has_value()) {
                cluster.server_commands[command_pair.second.value().name] = command_pair.second.value();
            }
        }
        // Iterate through all sdfEvents and map them individually
        for (const auto& sdf_event_pair : sdf_thing_pair.second.sdf_event) {
            current_quality_name_node = new ReferenceTreeNode("sdfEvent");
            sdf_thing_reference->AddChild(current_quality_name_node);
            cluster.events.push_back(MapSdfEvent(sdf_event_pair));
        }
        device.clusters.push_back(cluster);
    }

    return device;
}

//! Function used to map a sdf-model and a sdf-thing onto a device type as well as cluster definitions
int MapSdfToMatter(const sdf::SdfModel& sdf_model, const sdf::SdfMapping& sdf_mapping,
                   std::optional<matter::Device>& optional_device, std::list<matter::Cluster>& cluster_list) {
    // Make the mapping a global variable
    if (!sdf_mapping.map.empty()) {
        reference_map = sdf_mapping.map;
    }

    // Initialize a reference tree used to resolve json references
    ReferenceTree reference_tree;
    // Check if the model contains either sdfThings or sdfObject at the top level
    if (!sdf_model.sdf_thing.empty()){
        current_quality_name_node = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(current_quality_name_node);
        for (const auto& sdf_thing_pair : sdf_model.sdf_thing) {
            optional_device = MapSdfThing(sdf_thing_pair);
            if (optional_device.has_value()) {
                cluster_list = optional_device.value().clusters;
            }
        }
    }
    else if (!sdf_model.sdf_object.empty()){
        // Make sure, that optional_device is empty, as there is no sdfThing present
        optional_device.reset();
        current_quality_name_node = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(current_quality_name_node);
        // Iterate through all sdfObjects and add them to the list of clusters, after mapping them
        for (const auto& sdf_object_pair : sdf_model.sdf_object) {
            matter::Cluster cluster = MapSdfObject(sdf_object_pair);
            cluster_list.push_back(cluster);
        }
    }

    return 0;
}
