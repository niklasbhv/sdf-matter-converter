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
#include <utility>
#include "mapping.h"
#include "matter.h"
#include "sdf.h"

//! Map containing the elements of the sdf mapping
//! This map is used to resolve the elements outsourced into the map
std::map<std::string, std::map<std::string, sdf::MappingValue>> reference_map;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
std::list<std::string> sdf_required_list;

class ReferenceTreeNode {
public:
    std::string name;
    std::map<std::string, sdf::MappingValue> attributes;
    ReferenceTreeNode* parent;
    std::vector<ReferenceTreeNode*> children;

    ReferenceTreeNode(std::string name) : name(std::move(name)), attributes(), parent(nullptr) {}

    void AddChild(ReferenceTreeNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    void AddAttribute(std::string key, sdf::MappingValue value) {
        attributes[key] = std::move(value);
    }
};

class ReferenceTree {
public:
    ReferenceTreeNode* root;

    ReferenceTree() {
        root = new ReferenceTreeNode("#");
    }

    std::map<std::string, std::map<std::string, sdf::MappingValue>> GenerateMapping(ReferenceTreeNode* node) {
        std::map<std::string, std::map<std::string, sdf::MappingValue>> map;
        ReferenceTreeNode* current = node;
        for (const auto& child : current->children) {
            std::cout << "Current Child: " << child->name << std::endl;
            std::cout << "Child Path: " << GeneratePath(child) << std::endl;
            if (!child->attributes.empty()) {
                std::cout << "Found Attribute!" << std::endl;
                map[GeneratePath(child)] = child->attributes;
            }
            map.merge(GenerateMapping(child));
        }
        return map;
    }
private:
    std::string GeneratePath(ReferenceTreeNode* node) {
        std::string path;
        ReferenceTreeNode* current = node;
        while (current != nullptr) {
            path = current->name + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
    }
};

//! This is a global point to the current node
//! This is designed to point at the top level SDF element like
//! for example the `sdfThing` node, not a specific sdfThing
ReferenceTreeNode* current_node = nullptr;

//! For debug purposes, prints a visual representation of the tree
struct simple_walker : pugi::xml_tree_walker
{
    bool for_each(pugi::xml_node& node) override
    {
        for (int i = 0; i < depth(); ++i) std::cout << "--> "; // indentation
        std::cout << node.name() << std::endl;
        return true; // continue traversal
    }
};

// Function to escape JSON Pointer according to RFC 6901
std::string EscapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find('~', pos)) != std::string::npos) {
        result.replace(pos, 1, "~0");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find('/', pos)) != std::string::npos) {
        result.replace(pos, 1, "~1");
        pos += 2;
    }
    return result;
}

// Function to unescape JSON Pointer according to RFC 6901
std::string UnescapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find("~1", pos)) != std::string::npos) {
        result.replace(pos, 2, "/");
        pos += 1;
    }
    pos = 0;
    while ((pos = result.find("~0", pos)) != std::string::npos) {
        result.replace(pos, 2, "~");
        pos += 1;
    }
    return result;
}

// TODO: Check how this function should behave on an not available mapping
//! Generically typed for now as the mapping will later contain different types
//! @brief Imports given key value combination from the SDF Mapping file
//! @param name Name of the target field.
sdf::MappingValue ImportFromMapping(const std::string& name)
{
    sdf::MappingValue mapping_value;
    // Be aware that this function removes the first character from the reference string
    //result = reference_map.at(reference.substr(1)).at(entry);
    return mapping_value;
}

//! @brief Exports given key value combination to the SDF Mapping file.
//! @param name Name of the target field.
//! @param value Value for the target field.
void ExportToMapping(const std::string& name, const sdf::MappingValue& value)
{
    // reference_map[reference_tree.path()].insert({name, value});
}

/*
std::map<std::string, std::map<std::string, sdf::MappingValue>> GenerateMapping(const pugi::xml_node& node)
{
    std::map<std::string, std::map<std::string, sdf::MappingValue>> map;
    // If available, iterate through the attributes of a node
    std::map<std::string, sdf::MappingValue> attribute_map;
    for (auto attribute : node.attributes()) {
        attribute_map.insert({attribute.name(), attribute.value()});
    }

    // If one or more attributes are found insert them into the map
    if (!attribute_map.empty())
        // Remove the first slash as it is not needed
        // TODO: Cluster names can contain slashes (e.g. On/Off) which might render the function unusable
        // TODO: A potential workaround might be to encase such names like sdf_object/[CLUSTER_NAME]/sdf_action
        map.insert({node.path().substr(1), attribute_map});

    // Recursive call to iterate to all nodes in the tree
    for (auto child_node : node.children()) {
        map = GenerateMapping(child_node);
    }

    return map;
}*/

matter::Access GenerateMatterAccess()
{
    matter::Access access;
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
    return conformance;
}

//! Generates a Matter constraint with the information given by the data qualities
matter::Constraint GenerateMatterConstraint(const sdf::DataQuality& dataQuality)
{
    matter::Constraint constraint;
    //constraint.value = dataQuality.default_;
    if (dataQuality.type == "number" or dataQuality.type == "integer") {
        if (dataQuality.minimum.has_value())
            constraint.min = dataQuality.minimum.value();
        if (dataQuality.maximum.has_value())
            constraint.max = dataQuality.maximum.value();
        // exclusive_minimum
        // exclusive_maximum
        // multiple_of
    } else if (dataQuality.type == "string") {
        if (dataQuality.min_length.has_value())
            constraint.min = dataQuality.min_length.value();
        if (dataQuality.max_length.has_value())
            constraint.max = dataQuality.max_length.value();
        // pattern
        // format
    } else if (dataQuality.type == "array") {
        if (dataQuality.min_items.has_value())
            constraint.min = dataQuality.min_items.value();
        if (dataQuality.max_items.has_value())
            constraint.max = dataQuality.max_items.value();
        // unique_items
        // items -> Translate these into entry constraints
    } else if (dataQuality.type == "object") {
        // Currently does not seem like object contains usefull information for constraints
        // properties
        // required
    }
    return constraint;
}

/*
 * SDF data type -> Matter type
 */
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

    } else if (data_quality.type == "array") {

    } else if (data_quality.type == "object") {

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
    data_field.conformance = GenerateMatterConformance();
    //data_field.access
    data_field.summary = data_quality.description;
    //data_field.type =
    data_field.constraint = GenerateMatterConstraint(data_quality);
    data_field.quality;
    //data_field.default_ = data_quality.default_;
    return data_field;
}

matter::Event MapSdfEvent(const sdf::SdfEvent& sdf_event, pugi::xml_node& sdf_event_node)
{
    matter::Event event;
    auto current_event_node = sdf_event_node.append_child(sdf_event.label.c_str());
    //TODO: Event needs an ID, this needs to be set here
    event.name = sdf_event.label;
    event.summary = sdf_event.description;
    //comment TODO: Try to fit this into an XML Comment
    //sdf_ref TODO: Convert this to a global name instead of a path
    //sdf_required TODO: Collect these and set the conformance for the corresponding element to mandatory
    //sdf_output_data TODO: How do we map sdf_output_data to the Event Fields?
    for (auto elem : sdf_event.sdf_data) {
        MapSdfData(elem.second);
    }
    return event;
}

/*
 * sdf_input_data -> Matter field
 */
matter::DataField MapSdfInputData(const sdf::DataQuality& data_quality)
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
    //default
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

matter::Command MapSdfAction(const sdf::SdfAction& sdf_action, pugi::xml_node& sdf_action_node)
{
    matter::Command command;
    auto current_action_node = sdf_action_node.append_child(sdf_action.label.c_str());

    // TODO: As client and server commands are seperated, we have to create two new commands
    // TODO: Currently this only handles a single command
    // command.id
    command.name = sdf_action.label;
    // conformance
    // access
    command.summary = sdf_action.description;
    // default
    command.direction = "commandToServer";
    command.response = "N";

    // Map the sdf_input_data Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdf_action.sdf_input_data.has_value()) {
        if (sdf_action.sdf_input_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                command.command_fields.push_back(field);
            }
            //required
        } else {
            matter::DataField field = MapSdfInputData(sdf_action.sdf_input_data.value());
            command.command_fields.push_back(field);
        }
    }

    return command;
}

matter::Attribute MapSdfProperty(const sdf::SdfProperty& sdfProperty, pugi::xml_node& sdf_property_node)
{
    matter::Attribute attribute;
    auto current_property_node = sdf_property_node.append_child(sdfProperty.label.c_str());

    // TODO: Currently constraints are missing, they are essential for the mapping
    // attribute.id
    attribute.name = sdfProperty.label;
    // sdf_property.comment
    // sdf_property.sdf_ref
    // sdf_property.sdf_required
    // conformance
    attribute.access->write = sdfProperty.writable;
    attribute.access->read = sdfProperty.readable;
    // sdf_property.observable
    attribute.summary = sdfProperty.description;
    // TODO: Create type mapper from SDF to Matter
    // type
    //attribute.default_ = sdf_property.default_;
    //attribute.quality.nullable = sdf_property.nullable;
    // TODO: Check if this should in this case be set or ignored
    //attribute.quality.fixed = !sdf_property.const_.empty();

    return attribute;
}

matter::Cluster MapSdfObject(const sdf::SdfObject& sdf_object, pugi::xml_node& sdf_object_node)
{
    matter::Cluster cluster;
    auto current_object_node = sdf_object_node.append_child(sdf_object.label.c_str());
    // id
    //std::string id_variable;
    //ImportFromMapping(current_object_node.path(), "id", id_variable);
    //cluster.id = std::stoi(id_variable);
    cluster.name = sdf_object.label;
    // conformance
    // access
    cluster.summary = sdf_object.description;
    // revision
    // revision_history
    // classification

    // Iterate through all sdfProperties and parse them individually
    auto sdf_property_node = current_object_node.append_child("sdf_property");
    for (const auto& sdfProperty : sdf_object.sdf_property) {
        cluster.attributes.push_back(MapSdfProperty(sdfProperty.second, sdf_property_node));
    }

    // Iterate through all sdfActions and parse them individually
    auto sdf_action_node = current_object_node.append_child("sdf_action");
    for (const auto& sdfAction : sdf_object.sdf_action) {
        cluster.commands.push_back(MapSdfAction(sdfAction.second, sdf_action_node));
    }

    // Iterate through all sdfEvents and parse them individually
    auto sdf_event_node = current_object_node.append_child("sdf_event");
    for (const auto& sdfEvent : sdf_object.sdf_event) {
        cluster.events.push_back(MapSdfEvent(sdfEvent.second, sdf_event_node));
    }

    return cluster;
}

matter::Device MapSdfThing(const sdf::SdfThing& sdfThing, pugi::xml_node& sdf_thing_node)
{
    matter::Device device;
    // Add the current sdf_thing to the reference tree
    auto current_thing_node = sdf_thing_node.append_child(sdfThing.label.c_str());
    // TODO: Find a way to make the mapping compatible with multiple types
    // ImportFromMapping(current_thing_node.path(), "id" ,device.id);
    device.name = sdfThing.label;
    // conformance
    // access
    device.summary = sdfThing.description;
    // revision
    // revision_history

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdfObject : sdfThing.sdf_object) {
        auto sdf_object_node = current_thing_node.append_child("sdfObject");
        device.clusters.push_back(MapSdfObject(sdfObject.second, sdf_object_node));
    }

    return device;
}

/*
 * SDF-Model + SDF-Mapping -> List of Matter clusters<
 */
/*
int MapSdfToMatter(const sdf::SdfModel& sdfModel, const sdf::SdfMapping& sdfMappingType, std::list<matter::Cluster>& clusters)
{
    // Make the mapping a global variable
    if (!sdfMappingType.map.empty())
        reference_map = sdfMappingType.map;

    // Initialize a reference tree used to resolve json references
    //pugi::xml_document reference_tree;
    //auto reference_root_node = reference_tree.append_child("#");
    //auto sdf_object_node = reference_tree.append_child("sdfObject");

    for (const auto& sdf_object_pair : sdfModel.sdf_object) {
        //auto current_cluster_node = sdf_object_node.append_child(sdf_object_pair.first.c_str());
        //clusters.push_back(MapSdfObject(sdf_object_pair.second, current_cluster_node));
    }

    return 0;
}*/

/*
 * SDF-Model + SDF-Mapping -> Matter Device
 */
int MapSdfToMatter(const sdf::SdfModel& sdfModel,
                   const sdf::SdfMapping& sdfMappingType,
                   std::optional<matter::Device>& device, std::list<matter::Cluster>& clusters)
{
    // Make the mapping a global variable
    if (!sdfMappingType.map.empty())
        reference_map = sdfMappingType.map;

    // Initialize a reference tree used to resolve json references
    //pugi::xml_document reference_tree;
    //auto reference_root_node = reference_tree.append_child("#");

    // TODO: How do we handle multiple sdf_thing or sdf_object definitions
    if (!sdfModel.sdf_thing.empty()){
        //auto sdf_thing_node = reference_tree.append_child("sdfThing");
        //MapSdfThing(sdfModel.sdf_thing.value(), device, sdf_thing_node);
    } else if (!sdfModel.sdf_object.empty()){
        //auto sdf_object_node = reference_tree.append_child("sdfObject");
        // TODO: Special case, initialize a device with a single cluster
    }

    return 0;
}

//! Matter type -> SDF type
int MapMatterType(const std::string& matter_type, sdf::DataQuality& dataQuality)
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
int MapMatterConstraint(const matter::Constraint& constraint, sdf::DataQuality dataQuality)
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

//! Matter Access Type -> sdf_property
//! This function is used in combination with a sdf_property object
int MapMatterAccess(const matter::Access& access, sdf::SdfProperty& sdfProperty, pugi::xml_node& sdf_property_node)
{
    //! Most of the access qualities need to be moved to the mapping
    //TODO: Check if we should set default booleans here
    if (access.read.has_value())
        sdfProperty.readable = access.read;
    if (access.write.has_value())
        sdfProperty.writable = access.write;
    //sdf_property.observable
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
int MapMatterAccess(const matter::Access& access, pugi::xml_node& current_node)
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
 * @brief Adds element to sdf_required depending on the conformance.
 *
 * This function adds the current element to sdf_required it is set to mandatory via its conformance.
 *
 * @param conformance The conformance to check.
 * @param current_node The reference tree node of the current element.
 * @return 0 on success, negative on failure.
 */
int MapMatterConformance(const matter::Conformance& conformance) {
    if (conformance.mandatory.has_value()) {
        if (conformance.mandatory.value()) {
            //sdf_required_list.push_back(current_node.path().substr(1));
        }
    }
    // TODO: Currently there seems to be no way to handle conformance based on the selected feature
    // That's why the boolean expression is outsourced to the mapping file
    //if (conformance.condition.has_value()) {
    //    sdf_node.append_attribute("condition").set_value(conformance.condition.value().c_str());
    //}
    return 0;
}

sdf::SdfEvent MapMatterEvent(const matter::Event& event)
{
    sdf::SdfEvent sdf_event;
    // Append the event node to the tree
    //auto event_node = sdf_event_node.append_child(event.name.c_str());

    // event.id
    sdf_event.label = event.name;
    if (event.conformance.has_value())
        MapMatterConformance(event.conformance.value());
    // event.access
    sdf_event.description = event.summary;
    // event.priority
    // event.number
    // event.timestamp
    // event.data

    return sdf_event;
}

sdf::SdfAction MapMatterCommand(const matter::Command& client_command, matter::Command& server_command)
{
    sdf::SdfAction sdf_action;
    return sdf_action;
}

sdf::SdfAction MapMatterCommand(const matter::Command& client_command)
{
    sdf::SdfAction sdf_action;
    // Append the command node to the tree
    //auto command_node = sdf_action_node.append_child(client_command.name.c_str());

    // client_command.id
    sdf_action.label = client_command.name;
    if (client_command.conformance.has_value())
        MapMatterConformance(client_command.conformance.value());
    // client_command.access
    sdf_action.description = client_command.summary;
    // client_command.default_
    // client_command.direction
    // client_command.response

    return sdf_action;
}

sdf::SdfProperty MapMatterAttribute(const matter::Attribute& attribute)
{
    sdf::SdfProperty sdf_property;
    // Append the attribute node to the tree
    //auto attribute_node = sdf_property_node.append_child(attribute.name.c_str());
    auto* attribute_reference = new ReferenceTreeNode(attribute.name);
    current_node->AddChild(attribute_reference);
    attribute_reference->AddAttribute("id", (uint64_t) attribute.id);
    sdf_property.label = attribute.name;
    if (attribute.conformance.has_value())
        MapMatterConformance(attribute.conformance.value());
    // attribute.access
    sdf_property.description = attribute.summary;

    // Map the Matter type to a fitting SDF type
    MapMatterType(attribute.type, sdf_property);
    //if (attribute.quality.nullable.has_value())
        // TODO: In this case the boundaries for some types have to be changed
        //sdf_property.nullable = attribute.quality.nullable.value();
    // attribute.qualities.non_volatile
    // attribute.qualities.fixed
    // attribute.qualities.scene
    // attribute.qualities.reportable
    // attribute.qualities.changed_omitted
    // attribute.qualities.singleton
    //sdf_property.default_ = attribute.default_;

    return sdf_property;
}

sdf::SdfObject MapMatterCluster(const matter::Cluster& cluster)
{
    sdf::SdfObject sdf_object;
    auto* cluster_reference = new ReferenceTreeNode(cluster.name);

    cluster_reference->AddAttribute("id", (uint64_t) cluster.id);
    current_node->AddChild(cluster_reference);
    sdf_object.label = cluster.name;
    if (cluster.conformance.has_value())
        MapMatterConformance(cluster.conformance.value());
    // cluster.access
    sdf_object.description = cluster.summary;
    // cluster.revision -> sdf_data
    // cluster.revision_history -> sdf_data

    // Iterate through the attributes and map them
    auto* sdf_property_node = new ReferenceTreeNode("sdfProperty");
    cluster_reference->AddChild(sdf_property_node);
    current_node = sdf_property_node;
    for (const auto& attribute : cluster.attributes){
        sdf::SdfProperty sdf_property = MapMatterAttribute(attribute);
        sdf_object.sdf_property.insert({attribute.name, sdf_property});
    }

    // Iterate through the commands and map them
    //auto command_node = cluster_node.child("sdf_action");
    for (const auto& command : cluster.commands){
        sdf::SdfAction sdf_action = MapMatterCommand(command);
        sdf_object.sdf_action.insert({command.name, sdf_action});
    }

    // Iterate through the events and map them
    //auto event_node = cluster_node.child("sdf_event");
    for (const auto& event : cluster.events){
        sdf::SdfEvent sdf_event =  MapMatterEvent(event);
        sdf_object.sdf_event.insert({event.name, sdf_event});
    }

    return sdf_object;
}

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

sdf::SdfThing MapMatterDevice(const matter::Device& device)
{
    // Append a new sdf_object node to the tree
    //sdf_thing_node.append_child(device.name.c_str()).append_child("sdfObject");
    //auto device_node = sdf_thing_node.child(device.name.c_str());
    //device_node.append_attribute("id").set_value(device.id);
    // device.classification -> sdfMapping
    // device.features -> sdf_data
    // device.enums -> sdf_data
    // device.bitmaps -> sdf_data
    // device.structs -> sdf_data
    if (device.conformance.has_value())
        MapMatterConformance(device.conformance.value());
    // device.access
    // TODO: We need to be able to create a JSON object for more complex structures like revisionHistory
    // device.revisionHistory -> sdf_data

    sdf::SdfThing sdf_thing;
    sdf_thing.label = device.name;
    sdf_thing.description = device.summary;
    // sdf_thing.comment
    // sdf_thing.sdf_ref
    // sdf_thing.sdf_required
    // sdf_thing.sdf_property
    // sdf_thing.sdf_action
    // sdf_thing.sdf_event
    // sdf_thing.min_items
    // sdf_thing.max_items
    // Iterate through cluster definitions for the device
    //auto sdf_object_node = device_node.child("sdfObject");
    for (const auto& cluster : device.clusters){
        sdf::SdfObject sdf_object = MapMatterCluster(cluster);;
        sdf_thing.sdf_object.insert({cluster.name, sdf_object});
    }
    sdf_thing.sdf_required = sdf_required_list;
    //sdfModel.sdf_thing = sdf_thing;

    return sdf_thing;
}

int MapMatterToSdf(const std::optional<matter::Device>& device,
                   const std::list<matter::Cluster>& cluster_list,
                   sdf::SdfModel& sdf_model, sdf::SdfMapping& sdf_mapping)
{
    ReferenceTree reference_tree;
    current_node = new ReferenceTreeNode("sdfObject");
    reference_tree.root->AddChild(current_node);

    if (device.has_value()) {
        sdf_model.information_block = GenerateInformationBlock(device.value());
        sdf_mapping.information_block = GenerateInformationBlock(device.value());
    } else {
        for (const auto& cluster : cluster_list) {
            sdf_model.information_block = GenerateInformationBlock(cluster);
            sdf_mapping.information_block = GenerateInformationBlock(cluster);
            sdf::SdfObject sdf_object = MapMatterCluster(cluster);
            sdf_model.sdf_object.insert({sdf_object.label, sdf_object});
        }
    }

    // Initial sdf_mapping mapping
    //sdf_mapping.information_block.version = std::to_string(device.revision);

    //sdf_mapping.namespace_block.namespaces = {{"zcl", "https://zcl.example.com/sdf"}};
    //sdf_mapping.namespace_block.default_namespace = "zcl";
    sdf_mapping.map = reference_tree.GenerateMapping(reference_tree.root);
    std::cout << sdf_mapping.map.size() << std::endl;
    // Print the resulting tree
    //simple_walker walker;
    //referenceTree.traverse(walker);

    return 0;
}
