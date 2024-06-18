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
#include <regex>
#include <functional>
#include "mapping.h"
#include "matter.h"
#include "sdf.h"


/*
 * Map containing the elements of the sdf mapping
 * This map is used to resolve the elements outsourced into the map
 */
std::map<std::string, std::map<std::string, std::string>> reference_map;

/*
 * List containing required sdf elements
 * This list gets filled while mapping and afterward appended to the corresponding sdfModel
 */
std::list<std::string> sdf_required_list;

//! For debug purposes, prints a visual representation of the tree
struct simple_walker: pugi::xml_tree_walker
{
    bool for_each(pugi::xml_node& node) override
    {
        for (int i = 0; i < depth(); ++i) std::cout << "--> "; // indentation
        std::cout << node.name() << std::endl;
        return true; // continue traversal
    }
};

template <typename T>

// TODO: Check how this function should behave on an not available mapping
// Generically typed for now as the mapping will later contain different types
int resolve_mapping(const std::string& reference, const std::string& entry, T& result)
{
    // Be aware that this function removes the first character from the reference string
    result = reference_map.at(reference.substr(1)).at(entry);
    return 0;
}

/*
 * Generates a Matter conformance.
 */
int generate_matter_conformance()
{
    return 0;
}

/*
 * Generates a Matter constraint with the information given by the data qualities
 */
int generate_matter_constraint(const dataQualityType& dataQuality, matter::constraintType& constraint)
{
    //constraint.value = dataQuality.default_;
    if (dataQuality.type == "number" or dataQuality.type == "integer") {
        if (dataQuality.minimum.has_value())
            constraint.min = dataQuality.minimum.value();
        if (dataQuality.maximum.has_value())
            constraint.max = dataQuality.maximum.value();
        // exclusiveMinimum
        // exclusiveMaximum
        // multipleOf
    } else if (dataQuality.type == "string") {
        if (dataQuality.minLength.has_value())
            constraint.min = dataQuality.minLength.value();
        if (dataQuality.maxLength.has_value())
            constraint.max = dataQuality.maxLength.value();
        // pattern
        // format
    } else if (dataQuality.type == "array") {
        if (dataQuality.minItems.has_value())
            constraint.min = dataQuality.minItems.value();
        if (dataQuality.maxItems.has_value())
            constraint.max = dataQuality.maxItems.value();
        // uniqueItems
        // items -> Translate these into entry constraints
    } else if (dataQuality.type == "object") {
        // Currently does not seem like object contains usefull information for constraints
        // properties
        // required
    }
    return 0;
}

/*
 * SDF data type -> Matter type
 */
std::string map_sdf_data_type(const dataQualityType& dataQuality)
{
    std::string result;
    if (dataQuality.type == "number") {
        result = "double";
    } else if (dataQuality.type == "string") {
        result = "string";
    } else if (dataQuality.type == "boolean") {
        result = "bool";
    } else if (dataQuality.type == "integer") {

    } else if (dataQuality.type == "array") {

    } else if (dataQuality.type == "object") {

    } else if (dataQuality.sdfType == "byte-string") {
        result = "octstr";
    } else if (dataQuality.sdfType == "unix-time") {

    }
    return result;
}

//! sdfData -> Global Matter Data Definition
//! As Matter only supports defining data structures globally these all have to be added to the global list
int map_sdf_data(dataQualityType& dataQualities)
{
    return 0;
}

/**
 * @brief Map a sdfEvent to a Matter event.
 *
 *
 *
 * @param sdfEvent
 * @param event
 * @param sdf_event_node
 * @return
 */
int map_sdf_event(const sdfEventType& sdfEvent, matter::eventType& event, pugi::xml_node& sdf_event_node)
{
    auto current_event_node = sdf_event_node.append_child(sdfEvent.label.c_str());
    //TODO: Event needs an ID, this needs to be set here
    event.name = sdfEvent.label;
    event.summary = sdfEvent.description;
    //$comment TODO: Try to fit this into an XML Comment
    //sdfRef TODO: Convert this to a global name instead of a path
    //sdfRequired TODO: Collect these and set the conformance for the corresponding element to mandatory
    //sdfOutputData TODO: How do we map sdfOutputData to the Event Fields?
    for (auto elem : sdfEvent.sdfData) {
        map_sdf_data(elem.second);
    }
    return 0;
}

/*
 * sdfInputData -> Matter field
 */
int map_sdf_input_data(const dataQualityType& dataQuality, matter::dataFieldType& field)
{
    field.summary = dataQuality.description;
    field.name = dataQuality.label;
    //$comment
    //sdfRequired
    field.type = map_sdf_data_type(dataQuality);
    //sdfChoice
    //enum
    //const
    //default
    matter::constraintType constraint;
    generate_matter_constraint(dataQuality, constraint);
    field.constraint = constraint;
    //exclusiveMinimum
    //exclusiveMaximum
    //multipleOf
    //pattern
    //format
    //uniqueItems
    //items
    //unit
    //nullable
    //sdfType
    //contentFormat

    return 0;
}

/**
 * @brief Map a sdfAction to a Matter command.
 *
 *
 *
 * @param sdfAction
 * @param command
 * @param sdf_action_node
 * @return
 */
int map_sdf_action(const sdfActionType& sdfAction, matter::commandType& command, pugi::xml_node& sdf_action_node)
{
    auto current_action_node = sdf_action_node.append_child(sdfAction.label.c_str());

    // TODO: As client and server commands are seperated, we have to create two new commands
    // TODO: Currently this only handles a single command
    // command.id
    command.name = sdfAction.label;
    // conformance
    // access
    command.summary = sdfAction.description;
    // default
    command.direction = "commandToServer";
    command.response = "N";

    // Map the sdfInputData Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdfAction.sdfInputData.has_value()) {
        if (sdfAction.sdfInputData.value().type == "object") {
            for (const auto& quality_pair : sdfAction.sdfInputData.value().properties) {
                matter::dataFieldType field;
                map_sdf_input_data(quality_pair.second, field);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                command.command_fields.push_back(field);
            }
            //required
        } else {
            matter::dataFieldType field;
            map_sdf_input_data(sdfAction.sdfInputData.value(), field);
            command.command_fields.push_back(field);
        }
    }

    return 0;
}

//! sdfProperty -> Matter attribute
int map_sdf_property(const sdfPropertyType& sdfProperty, matter::attributeType& attribute, pugi::xml_node& sdf_property_node)
{
    auto current_property_node = sdf_property_node.append_child(sdfProperty.label.c_str());

    // TODO: Currently constraints are missing, they are essential for the mapping
    // attribute.id
    attribute.name = sdfProperty.label;
    // sdfProperty.$comment
    // sdfProperty.sdfRef
    // sdfProperty.sdfRequired
    // conformance
    attribute.access->write = sdfProperty.writable;
    attribute.access->read = sdfProperty.readable;
    // sdfProperty.observable
    attribute.summary = sdfProperty.description;
    // TODO: Create type mapper from SDF to Matter
    // type
    //attribute.default_ = sdfProperty.default_;
    //attribute.quality.nullable = sdfProperty.nullable;
    // TODO: Check if this should in this case be set or ignored
    //attribute.quality.fixed = !sdfProperty.const_.empty();

    return 0;
}

//! sdfObject -> Matter cluster
int map_sdf_object(const sdfObjectType& sdfObject, matter::clusterType& cluster, pugi::xml_node& sdf_object_node)
{
    auto current_object_node = sdf_object_node.append_child(sdfObject.label.c_str());
    // id
    //std::string id_variable;
    //resolve_mapping(current_object_node.path(), "id", id_variable);
    //cluster.id = std::stoi(id_variable);
    cluster.name = sdfObject.label;
    // conformance
    // access
    cluster.summary = sdfObject.description;
    // revision
    // revision_history
    // classification

    // Iterate through all sdfProperties and parse them individually
    auto sdf_property_node = current_object_node.append_child("sdfProperty");
    for (const auto& sdfProperty : sdfObject.sdfProperty) {
        matter::attributeType attribute;
        map_sdf_property(sdfProperty.second, attribute, sdf_property_node);
        cluster.attributes.push_back(attribute);
    }

    // Iterate through all sdfActions and parse them individually
    auto sdf_action_node = current_object_node.append_child("sdfAction");
    for (const auto& sdfAction : sdfObject.sdfAction) {
        matter::commandType command;
        map_sdf_action(sdfAction.second, command, sdf_action_node);
        cluster.commands.push_back(command);
    }

    // Iterate through all sdfEvents and parse them individually
    auto sdf_event_node = current_object_node.append_child("sdfEvent");
    for (const auto& sdfEvent : sdfObject.sdfEvent) {
        matter::eventType event;
        map_sdf_event(sdfEvent.second, event, sdf_event_node);
        cluster.events.push_back(event);
    }

    return 0;
}

//! sdfThing -> Matter device
int map_sdf_thing(const sdfThingType& sdfThing, matter::deviceType& device, pugi::xml_node& sdf_thing_node)
{
    // Add the current sdfThing to the reference tree
    auto current_thing_node = sdf_thing_node.append_child(sdfThing.label.c_str());
    // TODO: Find a way to make the mapping compatible with multiple types
    // resolve_mapping(current_thing_node.path(), "id" ,device.id);
    device.name = sdfThing.label;
    // conformance
    // access
    device.summary = sdfThing.description;
    // revision
    // revision_history

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdfObject : sdfThing.sdfObject) {
        matter::clusterType cluster;
        auto sdf_object_node = current_thing_node.append_child("sdfObject");
        map_sdf_object(sdfObject.second, cluster, sdf_object_node);
        device.clusters.push_back(cluster);
    }

    return 0;
}

/*
 * SDF-Model + SDF-Mapping -> List of Matter clusters<
 */
int map_sdf_to_matter(const sdfModelType& sdfModel, const sdfMappingType& sdfMappingType, std::list<matter::clusterType>& clusters)
{
    // Make the mapping a global variable
    if (!sdfMappingType.map.empty())
        reference_map = sdfMappingType.map;

    // Initialize a reference tree used to resolve json references
    pugi::xml_document reference_tree;
    auto reference_root_node = reference_tree.append_child("#");
    auto sdf_object_node = reference_tree.append_child("sdfObject");

    for (const auto& sdf_object_pair : sdfModel.sdfObject) {
        matter::clusterType cluster;
        auto current_cluster_node = sdf_object_node.append_child(sdf_object_pair.first.c_str());
        map_sdf_object(sdf_object_pair.second, cluster, current_cluster_node);
        clusters.push_back(cluster);
    }

    return 0;
}

/*
 * SDF-Model + SDF-Mapping -> Matter Device
 */
int map_sdf_to_matter(const sdfModelType& sdfModel, const sdfMappingType& sdfMappingType, matter::deviceType& device)
{
    // Make the mapping a global variable
    if (!sdfMappingType.map.empty())
        reference_map = sdfMappingType.map;

    // Initialize a reference tree used to resolve json references
    pugi::xml_document reference_tree;
    auto reference_root_node = reference_tree.append_child("#");

    // TODO: How do we handle multiple sdfThing or sdfObject definitions
    if (!sdfModel.sdfThing.empty()){
        auto sdf_thing_node = reference_tree.append_child("sdfThing");
        //map_sdf_thing(sdfModel.sdfThing.value(), device, sdf_thing_node);
    } else if (!sdfModel.sdfObject.empty()){
        auto sdf_object_node = reference_tree.append_child("sdfObject");
        // TODO: Special case, initialize a device with a single cluster
    }

    return 0;
}

//! Matter type -> SDF type
int map_matter_type(const std::string& matter_type, dataQualityType& dataQuality)
{
    // TODO: Custom types should be handled here
    // TODO: Maybe also set the default value here?
    if (matter_type == "bool") {
        dataQuality.type = "boolean";
    }
    else if (matter_type == "single") {
        dataQuality.type = "number";
    }
    else if (matter_type == "double") {
        dataQuality.type = "number";
    }
    else if (matter_type == "octstr") {
        dataQuality.type = "string";
    }
    else if (matter_type == "list") {
        dataQuality.type = "array";
    }
    else if (matter_type == "struct") {
        dataQuality.type = "object";
    }
    else if (matter_type.substr(0, 3) == "map") {
        dataQuality.type = "string";
    }
    else if (matter_type.substr(0, 3) == "int") {
        // TODO: These boundaries change if the corresponding value is nullable
        dataQuality.type = "integer";
        dataQuality.minimum = 0;
        if (matter_type.substr(3) == "8") {
            dataQuality.maximum = MATTER_U_INT_8_MAX;
        } else if (matter_type.substr(3) == "16") {
            dataQuality.maximum = MATTER_U_INT_16_MAX;
        } else if (matter_type.substr(3) == "24") {
            dataQuality.maximum = MATTER_U_INT_24_MAX;
        } else if (matter_type.substr(3) == "32") {
            dataQuality.maximum = MATTER_U_INT_32_MAX;
        } else if (matter_type.substr(3) == "40") {
            dataQuality.maximum = MATTER_U_INT_40_MAX;
        } else if (matter_type.substr(3) == "48") {
            dataQuality.maximum = MATTER_U_INT_48_MAX;
        } else if (matter_type.substr(3) == "56") {
            dataQuality.maximum = MATTER_U_INT_56_MAX;
        } else if (matter_type.substr(3) == "64") {
            dataQuality.maximum = MATTER_U_INT_64_MAX;
        }
    }
    else if (matter_type.substr(0, 4) == "uint") {
        dataQuality.type = "integer";
        if (matter_type.substr(4) == "8") {
            dataQuality.minimum = MATTER_INT_8_MIN;
            dataQuality.maximum = MATTER_INT_8_MAX;
        } else if (matter_type.substr(4) == "16") {
            dataQuality.minimum = MATTER_INT_16_MIN;
            dataQuality.maximum = MATTER_INT_16_MAX;
        } else if (matter_type.substr(4) == "24") {
            dataQuality.minimum = MATTER_INT_24_MIN;
            dataQuality.maximum = MATTER_INT_24_MAX;
        } else if (matter_type.substr(4) == "32") {
            dataQuality.minimum = MATTER_INT_32_MIN;
            dataQuality.maximum = MATTER_INT_32_MAX;
        } else if (matter_type.substr(4) == "40") {
            dataQuality.minimum = MATTER_INT_40_MIN;
            dataQuality.maximum = MATTER_INT_40_MAX;
        } else if (matter_type.substr(4) == "48") {
            dataQuality.minimum = MATTER_INT_48_MIN;
            dataQuality.maximum = MATTER_INT_48_MAX;
        } else if (matter_type.substr(4) == "56") {
            dataQuality.minimum = MATTER_INT_56_MIN;
            dataQuality.maximum = MATTER_INT_56_MAX;
        } else if (matter_type.substr(4) == "64") {
            dataQuality.minimum = MATTER_INT_64_MIN;
            dataQuality.maximum = MATTER_INT_64_MAX;
        }
    }

    // If the type is not a known standard type
    std::cout << "Found: " << matter_type << std::endl;
    return 0;
}

//! Matter Constraint -> Data Quality
int map_matter_constraint(const matter::constraintType& constraint, dataQualityType dataQuality)
{
    //TODO: The constraint depends on the given data type
    if (constraint.type == "desc") {
        // TODO: What do we do here?
    } else if (constraint.type == "between") {
        //if (constraint.range.has_value()) {
        //    dataQuality.minimum = std::get<0>(constraint.range.value());
        //    dataQuality.maximum = std::get<1>(constraint.range.value());
        //}
    } else if (constraint.type == "min") {
        if (constraint.min.has_value()) {
            dataQuality.minimum = constraint.min.value();
        }
    } else if (constraint.type == "max") {
        if (constraint.max.has_value()) {
            dataQuality.maximum = constraint.max.value();
        }
    }
    // TODO: This list is currently not exhaustive
    return 0;
}

//! Matter Access Type -> sdfProperty
//! This function is used in combination with a sdfProperty object
int map_matter_access(const matter::accessType& access, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node)
{
    //! Most of the access qualities need to be moved to the mapping
    //TODO: Check if we should set default booleans here
    if (access.read.has_value())
        sdfProperty.readable = access.read;
    if (access.write.has_value())
        sdfProperty.writable = access.write;
    //sdfProperty.observable
    if (access.fabric_scoped.has_value())
        sdf_property_node.append_attribute("fabricScoped").set_value(access.fabric_scoped.value());
    if (access.fabric_sensitive.has_value())
        sdf_property_node.append_attribute("fabricSensitive").set_value(access.fabric_sensitive.value());
    //if (access.requires_view_privilege.has_value())
    //    sdf_property_node.append_attribute("requiresViewPrivilege").set_value(access.requires_view_privilege.value());
    //if (access.requires_operate_privilege.has_value())
    //    sdf_property_node.append_attribute("requiresOperatePrivilege").set_value(access.requires_operate_privilege.value());
    //if (access.requires_manage_privilege.has_value())
    //    sdf_property_node.append_attribute("requiresManagePrivilege").set_value(access.requires_manage_privilege.value());
    //if (access.requires_administer_privilege.has_value())
    //    sdf_property_node.append_attribute("requiresAdministerPrivilege").set_value(access.requires_administer_privilege.value());
    if (access.timed.has_value())
        sdf_property_node.append_attribute("timed").set_value(access.timed.value());

    return 0;
}
//! Matter Access Type -> SDF Mapping
//! This function is used standalone to move all qualities to the SDF Mapping
int map_matter_access(const matter::accessType& access, pugi::xml_node& current_node)
{
    // TODO: Check if these are the actual field names from the xml
    if (access.read.has_value())
        current_node.append_attribute("read").set_value(access.read.value());
    if (access.write.has_value())
        current_node.append_attribute("write").set_value(access.write.value());
    if (access.fabric_scoped.has_value())
        current_node.append_attribute("fabricScoped").set_value(access.fabric_scoped.value());
    if (access.fabric_sensitive.has_value())
        current_node.append_attribute("fabricSensitive").set_value(access.fabric_sensitive.value());
    //if (access.requires_view_privilege.has_value())
    //    current_node.append_attribute("requiresViewPrivilege").set_value(access.requires_view_privilege.value());
    //if (access.requires_operate_privilege.has_value())
    //    current_node.append_attribute("requiresOperatePrivilege").set_value(access.requires_operate_privilege.value());
    //if (access.requires_manage_privilege.has_value())
    //    current_node.append_attribute("requiresManagePrivilege").set_value(access.requires_manage_privilege.value());
    //if (access.requires_administer_privilege.has_value())
    //    current_node.append_attribute("requiresAdministerPrivilege").set_value(access.requires_administer_privilege.value());
    if (access.timed.has_value())
        current_node.append_attribute("timed").set_value(access.timed.value());
    return 0;
}

// Function used to evaluate a string expression
std::function<bool()> buildEvaluator(const std::string& expr) {
    if (expr == "true") return []() { return true; };
    if (expr == "false") return []() { return false; };
    if (expr == "!true") return []() { return false; };
    if (expr == "!false") return []() { return true; };

    // Evaluate AND expressions
    size_t andPos = expr.find("&&");
    if (andPos != std::string::npos) {
        auto left = buildEvaluator(expr.substr(0, andPos));
        auto right = buildEvaluator(expr.substr(andPos + 2));
        return [left, right]() { return left() && right(); };
    }

    // Evaluate OR expressions
    size_t orPos = expr.find("||");
    if (orPos != std::string::npos) {
        auto left = buildEvaluator(expr.substr(0, orPos));
        auto right = buildEvaluator(expr.substr(orPos + 2));
        return [left, right]() { return left() || right(); };
    }

    throw std::runtime_error("Unrecognized expression: " + expr);
}


/**
 * @brief Adds element to sdfRequired depending on the conformance.
 *
 * This function adds the current element to sdfRequired it is set to mandatory via its conformance.
 *
 * @param conformance The conformance to check.
 * @param current_node The reference tree node of the current element.
 * @return 0 on success, negative on failure.
 */
int map_matter_conformance(const matter::conformanceType& conformance, const pugi::xml_node current_node, pugi::xml_node& sdf_node) {
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value()) {
            sdf_required_list.push_back(current_node.path().substr(1));
        }
    }
    // TODO: Currently there seems to be no way to handle conformance based on the selected feature
    // That's why the boolean expression is outsourced to the mapping file
    //if (conformance.condition.has_value()) {
    //    sdf_node.append_attribute("condition").set_value(conformance.condition.value().c_str());
    //}
    return 0;
}

//! Matter Event -> sdfEvent
int map_matter_event(const matter::eventType& event, sdfEventType& sdfEvent, pugi::xml_node& sdf_event_node)
{
    // Append the event node to the tree
    auto event_node = sdf_event_node.append_child(event.name.c_str());

    // event.id
    sdfEvent.label = event.name;
    if (event.conformance.has_value())
        map_matter_conformance(event.conformance.value(), event_node, sdf_event_node);
    // event.access
    sdfEvent.description = event.summary;
    // event.priority
    // event.number
    // event.timestamp
    // event.data

    return 0;
}

//! Matter Command -> sdfAction
//! Used if a client and a server command need to be processed
int map_matter_command(const matter::commandType& client_command, matter::commandType& server_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    return 0;
}

//! Matter Command -> sdfAction
//! Used if only a client command needs to be processed
int map_matter_command(const matter::commandType& client_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node)
{
    // Append the command node to the tree
    auto command_node = sdf_action_node.append_child(client_command.name.c_str());

    // client_command.id
    sdfAction.label = client_command.name;
    if (client_command.conformance.has_value())
        map_matter_conformance(client_command.conformance.value(), command_node, sdf_action_node);
    // client_command.access
    sdfAction.description = client_command.summary;
    // client_command.default_
    // client_command.direction
    // client_command.response

    return 0;
}

//! Matter Attribute -> sdfProperty
int map_matter_attribute(const matter::attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node)
{
    // Append the attribute node to the tree
    auto attribute_node = sdf_property_node.append_child(attribute.name.c_str());

    // attribute.id
    sdfProperty.label = attribute.name;
    if (attribute.conformance.has_value())
        map_matter_conformance(attribute.conformance.value(), attribute_node, sdf_property_node);
    // attribute.access
    sdfProperty.description = attribute.summary;

    // Map the Matter type to a fitting SDF type
    map_matter_type(attribute.type, sdfProperty);
    //if (attribute.quality.nullable.has_value())
        // TODO: In this case the boundaries for some types have to be changed
        //sdfProperty.nullable = attribute.quality.nullable.value();
    // attribute.qualities.non_volatile
    // attribute.qualities.fixed
    // attribute.qualities.scene
    // attribute.qualities.reportable
    // attribute.qualities.changed_omitted
    // attribute.qualities.singleton
    //sdfProperty.default_ = attribute.default_;

    return 0;
}

//! Matter Cluster -> sdfObject
int map_matter_cluster(const matter::clusterType& cluster, sdfObjectType& sdfObject, pugi::xml_node& sdf_object_node)
{
    // Append the name of the cluster to the tree
    // Also append sdfProperty, sdfAction and sdfEvent to the tree
    auto cluster_node = sdf_object_node.append_child(cluster.name.c_str());
    cluster_node.append_child("sdfProperty");
    cluster_node.append_child("sdfAction");
    cluster_node.append_child("sdfEvent");

    // cluster.id
    sdfObject.label = cluster.name;
    if (cluster.conformance.has_value())
        map_matter_conformance(cluster.conformance.value(), cluster_node, sdf_object_node);
    // cluster.access
    sdfObject.description = cluster.summary;
    // cluster.revision -> sdfData
    // cluster.revision_history -> sdfData

    // Iterate through the attributes and map them
    auto attribute_node = cluster_node.child("sdfProperty");
    for (const auto& attribute : cluster.attributes){
        sdfPropertyType sdfProperty;
        map_matter_attribute(attribute, sdfProperty, attribute_node);
        sdfObject.sdfProperty.insert({attribute.name, sdfProperty});
    }

    // Iterate through the commands and map them
    auto command_node = cluster_node.child("sdfAction");
    for (const auto& command : cluster.commands){
        sdfActionType sdfAction;
        map_matter_command(command, sdfAction, command_node);
        sdfObject.sdfAction.insert({command.name, sdfAction});
    }

    // Iterate through the events and map them
    auto event_node = cluster_node.child("sdfEvent");
    for (const auto& event : cluster.events){
        sdfEventType sdfEvent;
        map_matter_event(event, sdfEvent, event_node);
        sdfObject.sdfEvent.insert({event.name, sdfEvent});
    }

    return 0;
}

//! Matter Device -> SDF-Model (sdfThing)
int map_matter_device(const matter::deviceType& device, sdfModelType& sdfModel, pugi::xml_node& sdf_thing_node)
{
    // Append a new sdfObject node to the tree
    sdf_thing_node.append_child(device.name.c_str()).append_child("sdfObject");
    auto device_node = sdf_thing_node.child(device.name.c_str());
    device_node.append_attribute("id").set_value(device.id);
    // device.classification -> sdfMapping
    // device.features -> sdfData
    // device.enums -> sdfData
    // device.bitmaps -> sdfData
    // device.structs -> sdfData
    if (device.conformance.has_value())
        map_matter_conformance(device.conformance.value(), device_node, sdf_thing_node);
    // device.access
    // TODO: We need to be able to create a JSON object for more complex structures like revisionHistory
    // device.revisionHistory -> sdfData

    // Map the information block
    sdfModel.infoBlock.title = device.name;
    sdfModel.infoBlock.description = device.summary;
    sdfModel.infoBlock.version = std::to_string(device.revision);
    // sdfModel.infoBlock.modified
    // sdfModel.infoBlock.copyright <-- Maybe parse from the comment?
    // sdfModel.infoBlock.license <-- Maybe parse from the comment?
    // sdfModel.infoBlock.features <-- This can definitely be parsed from the features section
    // sdfModel.infoBlock.$comment

    // Map the namespace block
    sdfModel.namespaceBlock.namespaces.insert({"zcl", "https://zcl.example.com/sdf"});
    sdfModel.namespaceBlock.defaultNamespace = "zcl";

    // Map the definition block
    sdfThingType sdfThing;
    sdfThing.label = device.name;
    sdfThing.description = device.summary;
    // sdfThing.$comment
    // sdfThing.sdfRef
    // sdfThing.sdfRequired
    // sdfThing.sdfProperty
    // sdfThing.sdfAction
    // sdfThing.sdfEvent
    // sdfThing.minItems
    // sdfThing.maxItems
    // Iterate through cluster definitions for the device
    auto sdf_object_node = device_node.child("sdfObject");
    for (const auto& cluster : device.clusters){
        sdfObjectType sdfObject;
        map_matter_cluster(cluster, sdfObject, sdf_object_node);
        sdfThing.sdfObject.insert({cluster.name, sdfObject});
    }
    sdfThing.sdfRequired = sdf_required_list;
    //sdfModel.sdfThing = sdfThing;

    return 0;
}

//! Generates a valid mapping from the reference tree
int generate_mapping(const pugi::xml_node& node, std::map<std::string, std::map<std::string, std::string>>& map)
{
    // If available, iterate through the attributes of a node
    std::map<std::string, std::string> attribute_map;
    for (auto attribute : node.attributes()) {
        attribute_map.insert({attribute.name(), attribute.value()});
    }

    // If one or more attributes are found insert them into the map
    if (!attribute_map.empty())
        // Remove the first slash as it is not needed
        // TODO: Cluster names can contain slashes (e.g. On/Off) which might render the function unusable
        // TODO: A potential workaround might be to encase such names like sdfObject/[CLUSTER_NAME]/sdfAction
        map.insert({node.path().substr(1), attribute_map});

    // Recursive call to iterate to all nodes in the tree
    for (auto child_node : node.children()) {
        generate_mapping(child_node, map);
    }

    return 0;
}

int map_matter_to_sdf(const matter::clusterType& cluster, sdfModelType& sdfModel, sdfMappingType& sdfMapping)
{
    pugi::xml_document referenceTree;
    referenceTree.append_child("#").append_child("sdfObject");
    auto cluster_node = referenceTree.child("#").child("sdfObject");

    sdfObjectType sdfObject;
    map_matter_cluster(cluster, sdfObject, cluster_node);

    sdfModel.infoBlock.title = cluster.name;
    sdfModel.infoBlock.description = cluster.summary;

    sdfMapping.infoBlock.title = cluster.name;
    sdfMapping.infoBlock.description = cluster.summary;

    sdfModel.sdfObject.insert({cluster.name, sdfObject});

    return 0;
}

//! Matter -> SDF
int map_matter_to_sdf(const matter::deviceType& device, sdfModelType& sdfModel, sdfMappingType& sdfMapping)
{
    pugi::xml_document referenceTree;
    referenceTree.append_child("#").append_child("sdfThing");
    auto device_node = referenceTree.child("#").child("sdfThing");

    map_matter_device(device, sdfModel, device_node);

    // Initial sdfMapping mapping
    sdfMapping.infoBlock.title = device.name;
    sdfMapping.infoBlock.description = device.summary;
    sdfMapping.infoBlock.version = std::to_string(device.revision);
    sdfMapping.namespaceBlock.namespaces = {{"zcl", "https://zcl.example.com/sdf"}};
    sdfMapping.namespaceBlock.defaultNamespace = "zcl";
    std::map<std::string, std::map<std::string, std::string>> map;
    generate_mapping(referenceTree.document_element(), map);
    sdfMapping.map = map;

    // Print the resulting tree
    simple_walker walker;
    referenceTree.traverse(walker);

    return 0;
}


