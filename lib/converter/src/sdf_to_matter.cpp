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

//! Map containing the elements of the sdf mapping
//! This map is used to resolve the elements outsourced into the map
json reference_map;

bool CheckForRequired(const std::string& json_pointer)
{

    //if (sdf_required_list.)
    return true;
}

//! Generically typed for now as the mapping will later contain different types
//! @brief Imports given key value combination from the SDF Mapping file
//! @param name Name of the target field.
template <typename T> void ImportFromMapping(const std::string& json_pointer, const std::string& field, T& input)
{
    if (reference_map.contains(json_pointer)) {
        if (reference_map.at(json_pointer).contains(field)) {
            reference_map.at(json_pointer).at(field).get_to(input);
        }
    }
}

std::optional<matter::DefaultType> MapSdfDefaultValue(const sdf::VariableType& variable_type)
{
    if (std::holds_alternative<uint64_t>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<uint64_t>(variable_type);
        return default_type;
    }
    else if (std::holds_alternative<int64_t>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<int64_t>(variable_type);
        return default_type;
    }
    else if (std::holds_alternative<double>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<double>(variable_type);
        return default_type;
    }
    else if (std::holds_alternative<std::string>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<std::string>(variable_type);
        return default_type;
    }
    else if (std::holds_alternative<bool>(variable_type)) {
        matter::DefaultType default_type;
        default_type = std::get<bool>(variable_type);
        return default_type;
    }
    else if (std::holds_alternative<std::list<sdf::ArrayItem>>(variable_type)) {
        // Currently they do not seem to be really compatible with each other
        matter::DefaultType default_type;
        //default_type = to_string(std::get<std::list<sdf::ArrayItem>>(variable_type));
        return default_type;
    }
    return std::nullopt;
}

//! Imports the access information for the current object from the mapping
std::optional<matter::Access> ImportAccessFromMapping(const std::string& json_pointer)
{
    json access_json;
    ImportFromMapping(json_pointer, "access", access_json);
    if (access_json.is_null())
        return std::nullopt;
    matter::Access access;
    if (access_json.contains("read"))
        access_json.at("read").get_to(access.read);
    if (access_json.contains("write"))
        access_json.at("write").get_to(access.write);
    if (access_json.contains("fabricScoped"))
        access_json.at("fabricScoped").get_to(access.fabric_scoped);
    if (access_json.contains("fabricSensitive"))
        access_json.at("fabricSensitive").get_to(access.fabric_sensitive);
    if (access_json.contains("readPrivilege"))
        access_json.at("readPrivilege").get_to(access.read_privilege);
    if (access_json.contains("writePrivilege"))
        access_json.at("writePrivilege").get_to(access.write_privilege);
    if (access_json.contains("invokePrivilege"))
        access_json.at("invokePrivilege").get_to(access.invoke_privilege);
    if (access_json.contains("timed"))
        access_json.at("timed").get_to(access.timed);
    return access;
}

//! Import the other qualities information for the current object for the mapping
std::optional<matter::OtherQuality> ImportOtherQualityFromMapping(const std::string& json_pointer)
{
    json other_quality_json;
    ImportFromMapping(json_pointer, "quality", other_quality_json);
    if (other_quality_json.is_null())
        return std::nullopt;
    matter::OtherQuality other_quality;
    if (other_quality_json.contains("nullable"))
        other_quality_json.at("nullable").get_to(other_quality.nullable);
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
    if (other_quality_json.contains("scene"))
        other_quality_json.at("scene").get_to(other_quality.scene);
    if (other_quality_json.contains("reportable"))
        other_quality_json.at("reportable").get_to(other_quality.reportable);
    if (other_quality_json.contains("changeOmitted"))
        other_quality_json.at("changeOmitted").get_to(other_quality.change_omitted);
    if (other_quality_json.contains("singleton"))
        other_quality_json.at("singleton").get_to(other_quality.singleton);
    if (other_quality_json.contains("diagnostics"))
        other_quality_json.at("diagnostics").get_to(other_quality.diagnostics);
    if (other_quality_json.contains("largeMessage"))
        other_quality_json.at("largeMessage").get_to(other_quality.large_message);
    if (other_quality_json.contains("quieterReporting"))
        other_quality_json.at("quieterReporting").get_to(other_quality.quieter_reporting);
    return other_quality;
}

//! Generates a Matter access based on information of the provided sdfProperty
matter::Access GenerateMatterAccess(sdf::SdfProperty& sdf_property)
{
    matter::Access access;
    if (sdf_property.readable.has_value())
        access.read = sdf_property.readable;
    if (sdf_property.writable.has_value())
        access.write = sdf_property.writable;
    return access;
}

//! @brief Generates a Matter conformance.
//! The resulting conformance depends on the following factors:
//! - required quality for the object type
//! - being part of a sdfRequired quality
//! If the referred element mentioned in either of these factors,
//! a mandatory conformance will be created.
//! Otherwise a optional conformance will be created.
matter::Conformance GenerateMatterConformance()
{
    matter::Conformance conformance;
    json conformance_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "mandatoryConform", conformance_json);
    if (!conformance_json.is_null()) {
        conformance.mandatory = true;
        conformance.condition = conformance_json;
    }
    ImportFromMapping(current_given_name_node->GeneratePointer(), "optionalConform", conformance_json);
    if (!conformance_json.is_null()) {
        conformance.optional = true;
        conformance.condition = conformance_json;
    }
    ImportFromMapping(current_given_name_node->GeneratePointer(), "provisionalConform", conformance_json);
    if (!conformance_json.is_null()) {
        conformance.provisional = true;
    }
    ImportFromMapping(current_given_name_node->GeneratePointer(), "deprecateConform", conformance_json);
    if (!conformance_json.is_null()) {
        conformance.deprecated = true;
    }
    ImportFromMapping(current_given_name_node->GeneratePointer(), "disallowConform", conformance_json);
    if (!conformance_json.is_null()) {
        conformance.disallowed = true;
    }
    ImportFromMapping(current_given_name_node->GeneratePointer(), "otherwiseConformance", conformance_json);
    if (!conformance_json.is_null()) {}

    //if (CheckForRequired(""))
    //    conformance.mandatory = true;
    //else
    //    conformance.optional = true;

    return conformance;
}

//! Generates a Matter constraint with the information given by the data qualities
matter::Constraint GenerateMatterConstraint(const sdf::DataQuality& dataQuality)
{
    matter::Constraint constraint;
    //constraint.value = dataQuality.default_;
    if (dataQuality.type == "number" or dataQuality.type == "integer") {
        if (dataQuality.const_.has_value()) {
            //constraint.type = "allowed";
        }
        if (dataQuality.minimum.has_value()) {
            if (dataQuality.maximum.has_value()) {
                constraint.type = "between";
                constraint.min = dataQuality.minimum.value();
                constraint.max = dataQuality.maximum.value();
            } else {
                constraint.type = "min";
                constraint.min = dataQuality.minimum.value();
            }
        }
        else if (dataQuality.maximum.has_value()) {
            constraint.type = "max";
            constraint.max = dataQuality.maximum.value();
        }
    } else if (dataQuality.type == "string") {
        if (dataQuality.min_length.has_value()) {
            if (dataQuality.max_length.has_value()) {
                constraint.type = "lengthBetween";
                constraint.min = dataQuality.min_length.value();
                constraint.max = dataQuality.max_length.value();
            } else {
                constraint.type = "minLength";
                constraint.min = dataQuality.min_length.value();
            }
        }
        else if (dataQuality.max_length.has_value()) {
            constraint.type = "maxLength";
            constraint.max = dataQuality.max_length.value();
        }
    } else if (dataQuality.type == "array") {
        if (dataQuality.min_items.has_value()) {
            if (dataQuality.max_items.has_value()) {
                constraint.type = "countBetween";
                constraint.min = dataQuality.min_items.value();
                constraint.max = dataQuality.max_items.value();
            } else {
                constraint.type = "minCount";
                constraint.min = dataQuality.min_items.value();
            }
        } else if (dataQuality.max_items.has_value()) {
            constraint.type = "maxCount";
            constraint.max = dataQuality.max_items.value();
        }

        if (dataQuality.items.has_value()) {

        }
        // unique_items
        // items -> Translate these into entry constraints
    }
    return constraint;
}

// Function to check if the variant's integer value is within borders
bool CheckVariantBorders(const std::variant<double, int64_t, uint64_t>& variant, int64_t lowerBound, uint64_t upperBound) {
    return std::visit([&](auto&& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            if (value >= lowerBound && value <= upperBound) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }, variant);
}

std::string MapIntegerType(const sdf::DataQuality& data_quality)
{
    if (data_quality.minimum.has_value()) {
        // Check if the minimum value is positive or negative
        if (CheckVariantBorders(data_quality.minimum.value(), 0, std::numeric_limits<uint64_t>::max())) {
            if (data_quality.maximum.has_value()) {
                if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_8_MAX)) {
                    return "uint8";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_16_MAX)) {
                    return "uint16";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_24_MAX)) {
                    return "uint24";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_32_MAX)) {
                    return "uint32";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_40_MAX)) {
                    return "uint40";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_48_MAX)) {
                    return "uint48";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, MATTER_U_INT_56_MAX)) {
                    return "uint56";
                }
                else if (CheckVariantBorders(data_quality.maximum.value(), 0, std::numeric_limits<uint64_t>::max())) {
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
                        return "int8";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_16_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_16_MIN, MATTER_INT_16_MAX)) {
                        return "int16";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_24_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_24_MIN, MATTER_INT_24_MAX)) {
                        return "int24";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_32_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_32_MIN, MATTER_INT_32_MAX)) {
                        return "int32";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_40_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_40_MIN, MATTER_INT_40_MAX)) {
                        return "int40";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_48_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_48_MIN, MATTER_INT_48_MAX)) {
                        return "int48";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), MATTER_INT_56_MIN, 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), MATTER_INT_56_MIN, MATTER_INT_56_MAX)) {
                        return "int56";
                    }
                }
                else if (CheckVariantBorders(data_quality.minimum.value(), std::numeric_limits<int64_t>::max(), 0)) {
                    if (CheckVariantBorders(data_quality.maximum.value(), std::numeric_limits<int64_t>::min(),
                                            std::numeric_limits<int64_t>::max())) {
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

//! Determine a Matter type from the information's of a given data quality
std::string MapSdfDataType(const sdf::DataQuality& data_quality)
{
    std::string result;
    if (data_quality.type == "number") {
        result = "double";
    } else if (data_quality.type == "string") {
        result = "string";
    } else if (data_quality.type == "boolean") {
        result = "bool";
    } else if (data_quality.type == "integer") {
        result = MapIntegerType(data_quality);
    } else if (data_quality.type == "array") {

    } else if (data_quality.type == "object") {
        result = "struct";
    } else if (data_quality.sdf_type == "byte-string") {
        result = "octstr";
    } else if (data_quality.sdf_type == "unix-time") {

    }
    return result;
}

//! Maps a data quality onto a data field.
matter::DataField MapSdfData(sdf::DataQuality& data_quality)
{
    matter::DataField data_field;
    // data_field.id
    data_field.name = data_quality.label;
    //data_field.conformance = GenerateMatterConformance();
    // data_field.access
    data_field.summary = data_quality.description;
    data_field.type = MapSdfDataType(data_quality);
    data_field.constraint = GenerateMatterConstraint(data_quality);
    // data_field.quality;
    if (data_quality.default_.has_value())
        data_field.default_ = MapSdfDefaultValue(data_quality.default_.value());
    return data_field;
}

//! Maps either a sdfInputData or sdfOutputData element onto a Matter data field
matter::DataField MapSdfInputOutputData(const sdf::DataQuality& data_quality)
{
    matter::DataField data_field;
    data_field.summary = data_quality.description;
    data_field.name = data_quality.label;
    //comment
    //sdf_required
    data_field.type = MapSdfDataType(data_quality);
    //sdf_choice
    //enum
    //const
    if (data_quality.default_.has_value())
        data_field.default_ = MapSdfDefaultValue(data_quality.default_.value());
    data_field.constraint = GenerateMatterConstraint(data_quality);
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
matter::Event MapSdfEvent(const std::pair<std::string, sdf::SdfEvent>& sdf_event_pair)
{
    matter::Event event;
    auto* sdf_event_reference = new ReferenceTreeNode(sdf_event_pair.first);
    current_quality_name_node->AddChild(sdf_event_reference);
    current_given_name_node = sdf_event_reference;

    ImportFromMapping(sdf_event_reference->GeneratePointer(), "id", event.id);
    event.name = sdf_event_pair.second.label;
    event.summary = sdf_event_pair.second.description;
    event.conformance = GenerateMatterConformance();
    if (sdf_event_pair.second.sdf_output_data.has_value())
        MapSdfInputOutputData(sdf_event_pair.second.sdf_output_data.value());
    for (auto elem : sdf_event_pair.second.sdf_data) {
        MapSdfData(elem.second);
    }
    return event;
}

//! Maps a sdfAction onto a Matter client and optionally on a server command
std::pair<matter::Command, std::optional<matter::Command>> MapSdfAction(const std::pair<std::string, sdf::SdfAction>& sdf_action_pair)
{
    matter::Command client_command;
    auto* sdf_action_reference = new ReferenceTreeNode(sdf_action_pair.first);
    current_quality_name_node->AddChild(sdf_action_reference);
    current_given_name_node = sdf_action_reference;

    ImportFromMapping(sdf_action_reference->GeneratePointer(), "id", client_command.id);
    client_command.name = sdf_action_pair.second.label;
    // conformance
    client_command.access = ImportAccessFromMapping(sdf_action_reference->GeneratePointer());
    client_command.summary = sdf_action_pair.second.description;
    // default
    client_command.direction = "commandToServer";
    std::optional<matter::Command> optional_server_command;
    // Check if the sdfAction has output data qualities
    if (sdf_action_pair.second.sdf_output_data.has_value()) {
        //TODO: According to spec, the id should be different
        // Initially, we copy the contents of the client command
        matter::Command server_command = client_command;
        // If object is used as a type, the elements of the object have to be mapped individually
        if (sdf_action_pair.second.sdf_output_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                server_command.command_fields.push_back(field);
            }
        }
        else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_output_data.value());
            server_command.command_fields.push_back(field);
        }
        //required
        optional_server_command = server_command;
    } else {
        client_command.response = "N";
    }
    // TODO: Consider the status code here

    // Map the sdf_input_data Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdf_action_pair.second.sdf_input_data.has_value()) {
        // If object is used as a type, the elements of the object have to be mapped individually
        if (sdf_action_pair.second.sdf_input_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputOutputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                client_command.command_fields.push_back(field);
            }
            //required
        } else {
            matter::DataField field = MapSdfInputOutputData(sdf_action_pair.second.sdf_input_data.value());
            client_command.command_fields.push_back(field);
        }
    }

    return {client_command, optional_server_command};
}

//! Maps a sdfProperty onto a Matter attribute
matter::Attribute MapSdfProperty(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair)
{
    matter::Attribute attribute;
    auto* sdf_property_reference = new ReferenceTreeNode(sdf_property_pair.first);
    current_quality_name_node->AddChild(sdf_property_reference);
    current_given_name_node = sdf_property_reference;

    ImportFromMapping(sdf_property_reference->GeneratePointer(), "id", attribute.id);
    attribute.name = sdf_property_pair.second.label;
    // sdf_property.sdf_required
    attribute.conformance = GenerateMatterConformance();
    attribute.access = ImportAccessFromMapping(sdf_property_reference->GeneratePointer());
    attribute.access->write = sdf_property_pair.second.writable;
    attribute.access->read = sdf_property_pair.second.readable;
    // sdf_property.observable
    attribute.summary = sdf_property_pair.second.description;
    attribute.type = MapSdfDataType(sdf_property_pair.second);
    if (sdf_property_pair.second.default_.has_value())
        attribute.default_ = MapSdfDefaultValue(sdf_property_pair.second.default_.value());
    attribute.quality = ImportOtherQualityFromMapping(sdf_property_reference->GeneratePointer());
    //attribute.quality.nullable = sdf_property.nullable;
    //attribute.quality.fixed = !sdf_property.const_.empty();

    return attribute;
}

//! Imports the feature map for the current cluster from the mapping
std::list<matter::Feature> GenerateFeatureMap()
{
    std::list<matter::Feature> feature_map;
    json feature_map_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "features", feature_map_json);
    for (const auto& feature_json : feature_map_json) {
        matter::Feature feature;
        if (feature_json.contains("bit"))
            feature_json.at("bit").get_to(feature.bit);
        if (feature_json.contains("code"))
            feature_json.at("code").get_to(feature.code);
        if (feature_json.contains("name"))
            feature_json.at("name").get_to(feature.name);
        if (feature_json.contains("summary"))
            feature_json.at("summary").get_to(feature.summary);
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
matter::ClusterClassification GenerateClusterClassification()
{
    matter::ClusterClassification cluster_classification;
    json cluster_classification_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "classification", cluster_classification_json);
    if (cluster_classification_json.contains("hierarchy"))
        cluster_classification_json.at("hierarchy").get_to(cluster_classification.hierarchy);
    if (cluster_classification_json.contains("role"))
        cluster_classification_json.at("role").get_to(cluster_classification.role);
    if (cluster_classification_json.contains("pics_code"))
        cluster_classification_json.at("pics_code").get_to(cluster_classification.pics_code);
    if (cluster_classification_json.contains("scope"))
        cluster_classification_json.at("scope").get_to(cluster_classification.scope);
    if (cluster_classification_json.contains("baseCluster"))
        cluster_classification_json.at("baseCluster").get_to(cluster_classification.base_cluster);
    if (cluster_classification_json.contains("primaryTransaction"))
        cluster_classification_json.at("primaryTransaction").get_to(cluster_classification.primary_transaction);

    return cluster_classification;
}

//! Maps a sdfObject onto a Matter cluster
matter::Cluster MapSdfObject(const std::pair<std::string, sdf::SdfObject>& sdf_object_pair)
{
    matter::Cluster cluster;
    auto* sdf_object_reference = new ReferenceTreeNode(sdf_object_pair.first);
    current_quality_name_node->AddChild(sdf_object_reference);
    current_given_name_node = sdf_object_reference;

    ImportFromMapping(sdf_object_reference->GeneratePointer(), "id", cluster.id);
    cluster.name = sdf_object_pair.second.label;
    cluster.conformance = GenerateMatterConformance();
    cluster.summary = sdf_object_pair.second.description;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "side", cluster.side);
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "revision", cluster.revision);

    // Import the revision history from the mapping
    json revision_history_json;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "revisionHistory", revision_history_json);
    for (const auto& item : revision_history_json) {
        u_int8_t revision;
        item.at("revision").at("revision").get_to(revision);
        std::string summary;
        item.at("revision").at("summary").get_to(summary);
        cluster.revision_history[revision] = summary;
    }

    // Import the cluster aliases from the mapping
    json cluster_aliases_json;
    ImportFromMapping(sdf_object_reference->GeneratePointer(), "clusterIds", cluster_aliases_json);
    for (const auto& cluster_alias : cluster_aliases_json) {
        uint32_t id;
        cluster_alias.at("clusterId").at("id").get_to(id);
        std::string name;
        cluster_alias.at("clusterId").at("name").get_to(name);
        cluster.cluster_aliases.emplace_back(id, name);
    }

    cluster.classification = GenerateClusterClassification();

    cluster.feature_map = GenerateFeatureMap();

    // Iterate through all sdfProperties and parse them individually
    auto* sdf_property_reference = new ReferenceTreeNode("sdfProperty");
    sdf_object_reference->AddChild(sdf_property_reference);
    current_quality_name_node = sdf_property_reference;
    for (const auto& sdf_property_pair : sdf_object_pair.second.sdf_property) {
        cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
    }

    // Iterate through all sdfActions and parse them individually
    auto* sdf_action_reference = new ReferenceTreeNode("sdfAction");
    sdf_object_reference->AddChild(sdf_action_reference);
    current_quality_name_node = sdf_action_reference;
    for (const auto& sdf_action_pair : sdf_object_pair.second.sdf_action) {
        std::pair<matter::Command, std::optional<matter::Command>> command_pair = MapSdfAction(sdf_action_pair);
        cluster.client_commands.push_back(command_pair.first);
        if (command_pair.second.has_value())
            cluster.server_commands[command_pair.second.value().name] = command_pair.second.value();
    }

    // Iterate through all sdfEvents and parse them individually
    auto* sdf_event_reference = new ReferenceTreeNode("sdfEvent");
    sdf_object_reference->AddChild(sdf_event_reference);
    current_quality_name_node = sdf_event_reference;
    for (const auto& sdf_event_pair : sdf_object_pair.second.sdf_event) {
        cluster.events.push_back(MapSdfEvent(sdf_event_pair));
    }

    return cluster;
}

//! Imports the device classification for the current device type
matter::DeviceClassification GenerateDeviceClassification()
{
    matter::DeviceClassification device_classification;
    json device_classification_json;
    ImportFromMapping(current_given_name_node->GeneratePointer(), "classification", device_classification_json);
    if (device_classification_json.contains("superset"))
        device_classification_json.at("superset").get_to(device_classification.superset);
    if (device_classification_json.contains("class"))
        device_classification_json.at("class").get_to(device_classification.class_);
    if (device_classification_json.contains("scope"))
        device_classification_json.at("scope").get_to(device_classification.scope);
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
    device.summary = sdf_thing_pair.second.description;
    // Import the revision as well as the revision history from the mapping
    ImportFromMapping(sdf_thing_reference->GeneratePointer(), "revision", device.revision);
    json revision_history_json;
    ImportFromMapping(sdf_thing_reference->GeneratePointer(), "revisionHistory", revision_history_json);
    for (const auto& item : revision_history_json) {
        u_int8_t revision;
        item.at("revision").at("revision").get_to(revision);
        std::string summary;
        item.at("revision").at("summary").get_to(summary);
        device.revision_history[revision] = summary;
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
int MapSdfToMatter(const sdf::SdfModel& sdf_model,
                   const sdf::SdfMapping& sdf_mapping,
                   std::optional<matter::Device>& optional_device,
                   std::list<matter::Cluster>& cluster_list)
{
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
            if (optional_device.has_value())
                cluster_list = optional_device.value().clusters;
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
