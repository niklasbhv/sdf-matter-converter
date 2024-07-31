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

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
static std::list<std::string> sdf_required_list;

//! Map containing the elements of the sdf mapping
//! This map is used to resolve the elements outsourced into the map
json reference_map;

//! Function used to check, if the given pointer is part of a sdfRequired element
bool CheckForRequired(const std::string& json_pointer) {
    return contains(sdf_required_list, json_pointer);
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

//! @brief Generates a Matter conformance.
//! The resulting conformance depends on the following factors:
//! - required quality for the object type
//! - being part of a sdfRequired quality
//! If the referred element mentioned in either of these factors,
//! a mandatory conformance will be created.
//! Otherwise a optional conformance will be created.
matter::Conformance GenerateMatterConformance() {
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
        if (CheckForRequired(current_given_name_node->GeneratePointer())) {
            conformance.mandatory = true;
        } else {
            conformance.optional = true;
        }
    }
    return conformance;
}

//! @brief Generates a Matter conformance.
//! The resulting conformance depends on the following factors:
//! - required quality for the object type
//! - being part of a sdfRequired quality
//! If the referred element mentioned in either of these factors,
//! a mandatory conformance will be created.
//! Otherwise a optional conformance will be created.
matter::Conformance GenerateMatterConformance(json& conformance_json) {
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

    return conformance;
}

bool equals(int64_t a, uint64_t b) {
    if (a < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    auto ua = static_cast<uint64_t>(a);
    return ua == b;
}

bool equals(uint64_t a, int64_t b) {
    // Check if the int64_t value is negative
    if (b < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    // At this point, b is non-negative, so we can safely cast to uint64_t
    auto ub = static_cast<uint64_t>(b);
    return a == ub;
}

bool compare(int64_t a, uint64_t b) {
    // Check if the int64_t value is negative
    if (a < 0) {
        // A negative int64_t is always less than any uint64_t
        return true;
    }
    // At this point, a is non-negative, so we can safely cast to uint64_t
    auto ua = static_cast<uint64_t>(a);
    return ua <= b;
}

bool compare(uint64_t a, int64_t b) {
    // Check if the int64_t value is negative
    if (b < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    // At this point, b is non-negative, so we can safely cast to uint64_t
    auto ub = static_cast<uint64_t>(b);
    return a <= ub;
}

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

//! Function to check if the variant's integer value is within borders
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

std::string MapIntegerType(const sdf::DataQuality& data_quality, matter::Constraint& constraint) {
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
            else {}
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
            else {}
        }
    }
    if (data_quality.maximum.has_value()) {}
    return "";
}

std::string GetLastPartAfterSlash(const std::string& str) {
    size_t pos = str.find_last_of('/');
    if (pos != std::string::npos) {
        return str.substr(pos + 1);
    }
    return str;  // If no slash is found, return the original string
}

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

//! Determine a Matter type from the information's of a given data quality
std::string MapSdfDataType(const sdf::DataQuality& data_quality, matter::Constraint& constraint) {
    json desc_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "constraint", desc_json);

    if (desc_json.contains("type")) {
        desc_json.at("type").get_to(constraint.type);
    }

    std::string result;
    if (!data_quality.sdf_ref.empty()) {
        return GetLastPartAfterSlash(data_quality.sdf_ref);
    }

    if (data_quality.type == "number") {
        result = "double";
    } else if (data_quality.type == "string") {
        result = "string";
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
    } else if (data_quality.type == "boolean") {
        result = "bool";
    } else if (data_quality.type == "integer") {
        result = MapIntegerType(data_quality, constraint);
    } else if (data_quality.type == "array") {
        if (data_quality.items.has_value()) {
            auto* entry_constraint = new matter::Constraint();
            constraint.type = "entry";
            constraint.entry_type = MapSdfDataType(MapJsoItemToSdfDataQuality(data_quality.items.value()),
                                                   *entry_constraint);
            constraint.entry_constraint = entry_constraint;
        }
        result = "list";
    } else if (data_quality.type == "object") {
        result = "struct";
    } else if (data_quality.sdf_type == "byte-string") {
        result = "octstr";
    } else if (data_quality.sdf_type == "unix-time") {

    }

    return result;
}

//! Maps a data quality onto a data field.
matter::DataField MapSdfData(sdf::DataQuality& data_quality) {
    matter::DataField data_field;

    // data_field.id
    data_field.name = data_quality.label;
    //data_field.conformance = GenerateMatterConformance();
    // data_field.access
    data_field.summary = data_quality.description;
    matter::Constraint constraint;
    data_field.type = MapSdfDataType(data_quality, constraint);
    data_field.constraint = constraint;
    //data_field.constraint = GenerateMatterConstraint(data_quality);
    // data_field.quality;
    if (data_quality.default_.has_value()) {
        data_field.default_ = MapSdfDefaultValue(data_quality.default_.value());
    }

    return data_field;
}

//! Maps either a sdfInputData or sdfOutputData element onto a Matter data field
matter::DataField MapSdfInputOutputData(const sdf::DataQuality& data_quality) {
    matter::DataField data_field;

    data_field.summary = data_quality.description;
    data_field.name = data_quality.label;
    if (data_quality.nullable.has_value()) {
        matter::OtherQuality quality;
        quality.nullable = data_quality.nullable;
        data_field.quality = quality;
    }
    //comment
    //sdf_required
    matter::Constraint constraint;
    data_field.type = MapSdfDataType(data_quality, constraint);
    data_field.constraint = constraint;
    //sdf_choice
    //enum
    //const
    if (data_quality.default_.has_value()) {
        data_field.default_ = MapSdfDefaultValue(data_quality.default_.value());
    }
    //exclusive_minimum
    //exclusive_maximum
    //multiple_of
    //pattern
    //format
    //unique_items
    //items
    //unit
    //nullable
    //sdf_type
    //content_format

    return data_field;
}

//! Maps a sdfEvent onto a Matter event
matter::Event MapSdfEvent(const std::pair<std::string, sdf::SdfEvent>& sdf_event_pair) {
    matter::Event event;
    auto* sdf_event_reference = new ReferenceTreeNode(sdf_event_pair.first);
    current_quality_name_node->AddChild(sdf_event_reference);
    current_given_name_node = sdf_event_reference;

    ImportFromMapping(sdf_event_reference->GeneratePointer(), "id", event.id);
    event.name = sdf_event_pair.second.label;
    event.summary = sdf_event_pair.second.description;
    event.conformance = GenerateMatterConformance();
    if (sdf_event_pair.second.sdf_output_data.has_value()) {
        MapSdfInputOutputData(sdf_event_pair.second.sdf_output_data.value());
    }
    for (auto elem : sdf_event_pair.second.sdf_data) {
        MapSdfData(elem.second);
    }
    return event;
}

//! Maps a sdfAction onto a Matter client and optionally on a server command
std::pair<matter::Command, std::optional<matter::Command>> MapSdfAction(const std::pair<std::string,
                                                                        sdf::SdfAction>& sdf_action_pair) {
    matter::Command client_command;
    auto* sdf_action_reference = new ReferenceTreeNode(sdf_action_pair.first);
    current_quality_name_node->AddChild(sdf_action_reference);
    current_given_name_node = sdf_action_reference;

    ImportFromMapping(sdf_action_reference->GeneratePointer(), "id", client_command.id);
    client_command.name = sdf_action_pair.second.label;
    client_command.conformance = GenerateMatterConformance();
    client_command.access = ImportAccessFromMapping(sdf_action_reference->GeneratePointer());
    client_command.summary = sdf_action_pair.second.description;
    // default
    client_command.direction = "commandToServer";
    std::optional<matter::Command> optional_server_command;
    // Check if the sdfAction has output data qualities
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
            server_command.conformance = GenerateMatterConformance();
            server_command.summary = sdf_action_pair.second.sdf_output_data.value().description;
            server_command.direction = "responseFromServer";

            client_command.response = server_command.name;
            // If object is used as a type, the elements of the object have to be mapped individually
            if (sdf_action_pair.second.sdf_output_data.value().type == "object") {
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
            } else  {
                matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_output_data.value());
                json conformance_json;
                if (ImportFromMapping(sdf_action_reference->GeneratePointer(), "field", conformance_json)) {
                    field.conformance = GenerateMatterConformance(conformance_json);
                }
                field.id = 0;
                server_command.command_fields.push_back(field);
            }
            //required
            optional_server_command = server_command;
        }
    } else {
        client_command.response = "N";
    }

    // Map the sdf_input_data Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdf_action_pair.second.sdf_input_data.has_value()) {
        // If object is used as a type, the elements of the object have to be mapped individually
        if (sdf_action_pair.second.sdf_input_data.value().type == "object") {
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
            //required
        } else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_input_data.value());
            json conformance_json;
            if (ImportFromMapping(sdf_action_reference->GeneratePointer(), "field", conformance_json)) {
                field.conformance = GenerateMatterConformance(conformance_json);
            }
            field.id = 0;
            client_command.command_fields.push_back(field);
        }
    }

    return {client_command, optional_server_command};
}

//! Maps a sdfProperty onto a Matter attribute
matter::Attribute MapSdfProperty(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair) {
    matter::Attribute attribute;
    auto* sdf_property_reference = new ReferenceTreeNode(sdf_property_pair.first);
    current_quality_name_node->AddChild(sdf_property_reference);
    current_given_name_node = sdf_property_reference;

    ImportFromMapping(sdf_property_reference->GeneratePointer(), "id", attribute.id);
    attribute.name = sdf_property_pair.first;
    attribute.conformance = GenerateMatterConformance();

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
    attribute.constraint = constraint;
    if (sdf_property_pair.second.default_.has_value()) {
        attribute.default_ = MapSdfDefaultValue(sdf_property_pair.second.default_.value());
    } else {
        ImportFromMapping(current_given_name_node->GeneratePointer(), "default", attribute.default_);
    }

    return attribute;
}

//! Imports the feature map for the current cluster from the mapping
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

//! Imports the cluster classification for the current cluster from the mapping
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

//! Maps a sdfObject onto a Matter cluster
matter::Cluster MapSdfObject(const std::pair<std::string, sdf::SdfObject>& sdf_object_pair) {
    matter::Cluster cluster;
    auto* sdf_object_reference = new ReferenceTreeNode(sdf_object_pair.first);
    current_quality_name_node->AddChild(sdf_object_reference);
    current_given_name_node = sdf_object_reference;

    ImportFromMapping(sdf_object_reference->GeneratePointer(), "id", cluster.id);
    cluster.name = sdf_object_pair.second.label;
    if (!sdf_object_pair.second.sdf_required.empty()) {
        sdf_required_list.insert(sdf_required_list.end(),
                                 sdf_object_pair.second.sdf_required.begin(),
                                 sdf_object_pair.second.sdf_required.end());
    }
    cluster.conformance = GenerateMatterConformance();
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
        cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
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

    for (const auto& sdf_data_elem : sdf_object_pair.second.sdf_data) {
        auto* given_sdf_data_reference = new ReferenceTreeNode(sdf_data_elem.first);
        sdf_data_reference->AddChild(given_sdf_data_reference);
        current_given_name_node = given_sdf_data_reference;

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
                        data_field.conformance = GenerateMatterConformance(conformance_json.at(property.first));
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
                                bitfield.conformance = GenerateMatterConformance(bitfield_json);
                            }
                        }
                    }
                    bitmap.push_back(bitfield);
                }
                cluster.bitmaps[sdf_data_elem.first] = bitmap;
            }
        } else if (!sdf_data_elem.second.sdf_choice.empty()) {
            std::list<matter::Item> matter_enum;
            int value = 0;
            for (const auto& sdf_choice : sdf_data_elem.second.sdf_choice) {
                matter::Item item;
                item.name = sdf_choice.first;
                item.summary = sdf_choice.second.description;
                json conformance_json;
                if (sdf_choice.second.const_.has_value()) {
                    if (std::holds_alternative<double>(sdf_choice.second.const_.value())) {
                        item.value = static_cast<int>(std::get<double>(sdf_choice.second.const_.value()));
                    } else if (std::holds_alternative<uint64_t>(sdf_choice.second.const_.value())) {
                        item.value = static_cast<int>(std::get<uint64_t>(sdf_choice.second.const_.value()));
                    } else if (std::holds_alternative<int64_t>(sdf_choice.second.const_.value())) {
                        item.value = static_cast<int>(std::get<int64_t>(sdf_choice.second.const_.value()));
                    }
                } else {
                    item.value = value;
                    value++;
                }
                if (ImportFromMapping(current_given_name_node->GeneratePointer(), "item", conformance_json)) {
                    for (auto& item_json : conformance_json) {
                        if (item_json.at("value") == item.value) {
                            item.conformance = GenerateMatterConformance(item_json);
                            break;
                        }
                    }
                }
                matter_enum.push_back(item);
            }
            cluster.enums[sdf_data_elem.first] = matter_enum;
        }
    }

    return cluster;
}

//! Imports the device classification for the current device type
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

//! Maps a sdfThing onto a Matter device type
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

    return device;
}

//! Creates Matter optional_device as well as cluster definitions from a given SDF Model and SDF Mapping
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
