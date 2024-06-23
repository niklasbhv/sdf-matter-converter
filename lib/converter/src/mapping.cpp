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
//std::map<std::string, std::map<std::string, sdf::MappingValue>> reference_map;
json reference_map;

//! List containing required sdf elements
//! This list gets filled while mapping and afterward appended to the corresponding sdfModel
std::list<std::string> sdf_required_list;

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

    std::string GeneratePointer() {
        std::string path;
        ReferenceTreeNode* current = this;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
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
            if (!child->attributes.empty()) {
                map[GeneratePointer(child)] = child->attributes;
            }
            map.merge(GenerateMapping(child));
        }
        return map;
    }

    std::string GeneratePointer(ReferenceTreeNode* node) {
        std::string path;
        ReferenceTreeNode* current = node;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
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

//! Generically typed for now as the mapping will later contain different types
//! @brief Imports given key value combination from the SDF Mapping file
//! @param name Name of the target field.
/*
std::optional<sdf::MappingValue> ImportFromMapping(const std::string& json_pointer, const std::string& field)
{
    if (reference_map.count(json_pointer)) {
        if (reference_map.at(json_pointer).count(field)) {
            return reference_map.at(json_pointer).at(field);
        }
    }
    return std::nullopt;
}*/

template <typename T> void ImportFromMapping(const std::string& json_pointer, const std::string& field, T& input)
{
    if (!reference_map.at(json_pointer).is_null()) {
        if (!reference_map.at(json_pointer).at(field).is_null()) {
            reference_map.at(json_pointer).at(field).get_to(input);
        }
    }
}

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

//! Generate a Matter type from the information's of the given data quality
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
    data_field.conformance = GenerateMatterConformance();
    // data_field.access
    data_field.summary = data_quality.description;
    // data_field.type =
    data_field.constraint = GenerateMatterConstraint(data_quality);
    // data_field.quality;
    // data_field.default_ = data_quality.default_;
    return data_field;
}

matter::Event MapSdfEvent(const std::pair<std::string, sdf::SdfEvent>& sdf_event_pair)
{
    matter::Event event;
    auto* sdf_event_reference = new ReferenceTreeNode(sdf_event_pair.first);
    current_node->AddChild(sdf_event_reference);
    //TODO: Event needs an ID, this needs to be set here
    event.name = sdf_event_pair.second.label;
    event.summary = sdf_event_pair.second.description;
    //comment TODO: Try to fit this into an XML Comment
    //sdf_required TODO: Collect these and set the conformance for the corresponding element to mandatory
    //sdf_output_data TODO: How do we map sdf_output_data to the Event Fields?
    for (auto elem : sdf_event_pair.second.sdf_data) {
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

matter::Command MapSdfAction(const std::pair<std::string, sdf::SdfAction>& sdf_action_pair)
{
    matter::Command command;
    auto* sdf_action_reference = new ReferenceTreeNode(sdf_action_pair.first);
    current_node->AddChild(sdf_action_reference);

    // TODO: As client and server commands are seperated, we have to create two new commands
    // TODO: Currently this only handles a single command
    // command.id
    command.name = sdf_action_pair.second.label;
    // conformance
    // access
    command.summary = sdf_action_pair.second.description;
    // default
    command.direction = "commandToServer";
    command.response = "N";

    // Map the sdf_input_data Qualities
    // If object is used as a type, the elements of the object have to be mapped individually
    if (sdf_action_pair.second.sdf_input_data.has_value()) {
        if (sdf_action_pair.second.sdf_input_data.value().type == "object") {
            for (const auto& quality_pair : sdf_action_pair.second.sdf_input_data.value().properties) {
                matter::DataField field = MapSdfInputData(quality_pair.second);
                // If no label is given, set the quality name
                if (field.name.empty())
                    field.name = quality_pair.first;
                command.command_fields.push_back(field);
            }
            //required
        } else {
            matter::DataField field = MapSdfInputData(sdf_action_pair.second.sdf_input_data.value());
            command.command_fields.push_back(field);
        }
    }

    return command;
}

matter::Attribute MapSdfProperty(const std::pair<std::string, sdf::SdfProperty>& sdf_property_pair)
{
    matter::Attribute attribute;
    auto* sdf_property_reference = new ReferenceTreeNode(sdf_property_pair.first);
    current_node->AddChild(sdf_property_reference);

    ImportFromMapping(sdf_property_reference->GeneratePointer(), "id", attribute.id);
    attribute.name = sdf_property_pair.second.label;
    // sdf_property.comment
    // sdf_property.sdf_ref
    // sdf_property.sdf_required
    // conformance
    attribute.access->write = sdf_property_pair.second.writable;
    attribute.access->read = sdf_property_pair.second.readable;
    // sdf_property.observable
    attribute.summary = sdf_property_pair.second.description;
    attribute.type = MapSdfDataType(sdf_property_pair.second);
    //attribute.default_ = sdf_property.default_;
    //attribute.quality.nullable = sdf_property.nullable;
    // TODO: Check if this should in this case be set or ignored
    //attribute.quality.fixed = !sdf_property.const_.empty();

    return attribute;
}

matter::Cluster MapSdfObject(const std::pair<std::string, sdf::SdfObject>& sdf_object_pair)
{
    matter::Cluster cluster;
    auto* sdf_object_reference = new ReferenceTreeNode(sdf_object_pair.first);
    current_node->AddChild(sdf_object_reference);

    ImportFromMapping(sdf_object_reference->GeneratePointer(), "id", cluster.id);
    cluster.name = sdf_object_pair.second.label;
    // conformance
    // access
    cluster.summary = sdf_object_pair.second.description;
    // revision
    // revision_history
    // classification

    // Iterate through all sdfProperties and parse them individually
    auto* sdf_property_reference = new ReferenceTreeNode("sdfProperty");
    sdf_object_reference->AddChild(sdf_property_reference);
    current_node = sdf_property_reference;
    for (const auto& sdf_property_pair : sdf_object_pair.second.sdf_property) {
        cluster.attributes.push_back(MapSdfProperty(sdf_property_pair));
    }

    // Iterate through all sdfActions and parse them individually
    auto* sdf_action_reference = new ReferenceTreeNode("sdfAction");
    sdf_object_reference->AddChild(sdf_action_reference);
    current_node = sdf_action_reference;
    for (const auto& sdf_action_pair : sdf_object_pair.second.sdf_action) {
        cluster.commands.push_back(MapSdfAction(sdf_action_pair));
    }

    // Iterate through all sdfEvents and parse them individually
    auto* sdf_event_reference = new ReferenceTreeNode("sdfEvent");
    sdf_object_reference->AddChild(sdf_event_reference);
    current_node = sdf_event_reference;
    for (const auto& sdf_event_pair : sdf_object_pair.second.sdf_event) {
        cluster.events.push_back(MapSdfEvent(sdf_event_pair));
    }

    return cluster;
}

matter::Device MapSdfThing(const std::pair<std::string, sdf::SdfThing>& sdf_thing_pair)
{
    matter::Device device;
    // Add the current sdf_thing to the reference tree
    auto* sdf_thing_reference = new ReferenceTreeNode(sdf_thing_pair.first);
    current_node->AddChild(sdf_thing_reference);
    // Import the ID from the mapping
    ImportFromMapping(sdf_thing_reference->GeneratePointer(), "id", device.id);
    device.name = sdf_thing_pair.second.label;
    // conformance
    // access
    device.summary = sdf_thing_pair.second.description;
    // revision
    // revision_history

    // Iterate through all sdfObjects and map them individually
    for (const auto& sdf_object_pair : sdf_thing_pair.second.sdf_object) {
        current_node = new ReferenceTreeNode("sdfObject");
        sdf_thing_reference->AddChild(current_node);
        device.clusters.push_back(MapSdfObject(sdf_object_pair));
    }

    return device;
}

//! Creates Matter device as well as cluster definitions from a given SDF Model and SDF Mapping
int MapSdfToMatter(const sdf::SdfModel& sdf_model,
                   const sdf::SdfMapping& sdf_mapping,
                   std::optional<matter::Device>& device,
                   std::list<matter::Cluster>& cluster_list)
{
    // Make the mapping a global variable
    if (!sdf_mapping.map.empty()) {
        reference_map = sdf_mapping.map;
    }


    // Initialize a reference tree used to resolve json references
    ReferenceTree reference_tree;

    if (!sdf_model.sdf_thing.empty()){
        current_node = new ReferenceTreeNode("sdfThing");
        reference_tree.root->AddChild(current_node);
        for (const auto& sdf_thing_pair : sdf_model.sdf_thing) {
            // TODO: Should we consider multiple sdfThing definitions?
            device = MapSdfThing(sdf_thing_pair);
        }
    } else if (!sdf_model.sdf_object.empty()){
        // Make sure, that device is empty, as there is no sdfThing present
        device.reset();
        current_node = new ReferenceTreeNode("sdfObject");
        reference_tree.root->AddChild(current_node);
        for (const auto& sdf_object_pair : sdf_model.sdf_object) {
            matter::Cluster cluster = MapSdfObject(sdf_object_pair);
            cluster_list.push_back(cluster);
        }
    }

    return 0;
}

void MapOtherQuality(const matter::OtherQuality& other_quality, sdf::SdfProperty& sdf_property)
{
    /*
    if (other_quality.nullable.has_value())
    if (other_quality.non_volatile.has_value())
    if (other_quality.fixed.has_value())
    if (other_quality.scene.has_value())
    if (other_quality.reportable.has_value())
    if (other_quality.change_omitted.has_value())
    if (other_quality.singleton.has_value())
    if (other_quality.diagnostics.has_value())
    if (other_quality.large_message.has_value())
    if (other_quality.quieter_reporting.has_value())
     */
}

//! Generates data qualities based on the given matter type
void MapMatterType(const std::string& matter_type, sdf::DataQuality& data_quality)
{
    // TODO: Custom types should be handled here
    // TODO: Maybe also set the default value here?
    if (matter_type == "bool") {
        data_quality.type = "boolean";
    }
    else if (matter_type == "single") {
        data_quality.type = "number";
    }
    else if (matter_type == "double") {
        data_quality.type = "number";
    }
    else if (matter_type == "octstr") {
        data_quality.type = "string";
        data_quality.sdf_type = "byte-string";
    }
    else if (matter_type == "list") {
        data_quality.type = "array";
    }
    else if (matter_type == "struct") {
        data_quality.type = "object";
    }
    else if (matter_type.substr(0, 3) == "map") {
        data_quality.type = "string";
    }
    else if (matter_type.substr(0, 3) == "int") {
        // TODO: These boundaries change if the corresponding value is nullable
        data_quality.type = "integer";
        data_quality.minimum = 0;
        if (matter_type.substr(3) == "8") {
            data_quality.maximum = MATTER_U_INT_8_MAX;
        } else if (matter_type.substr(3) == "16") {
            data_quality.maximum = MATTER_U_INT_16_MAX;
        } else if (matter_type.substr(3) == "24") {
            data_quality.maximum = MATTER_U_INT_24_MAX;
        } else if (matter_type.substr(3) == "32") {
            data_quality.maximum = MATTER_U_INT_32_MAX;
        } else if (matter_type.substr(3) == "40") {
            data_quality.maximum = MATTER_U_INT_40_MAX;
        } else if (matter_type.substr(3) == "48") {
            data_quality.maximum = MATTER_U_INT_48_MAX;
        } else if (matter_type.substr(3) == "56") {
            data_quality.maximum = MATTER_U_INT_56_MAX;
        } else if (matter_type.substr(3) == "64") {
            data_quality.maximum = MATTER_U_INT_64_MAX;
        }
    }
    else if (matter_type.substr(0, 4) == "uint") {
        data_quality.type = "integer";
        if (matter_type.substr(4) == "8") {
            data_quality.minimum = MATTER_INT_8_MIN;
            data_quality.maximum = MATTER_INT_8_MAX;
        } else if (matter_type.substr(4) == "16") {
            data_quality.minimum = MATTER_INT_16_MIN;
            data_quality.maximum = MATTER_INT_16_MAX;
        } else if (matter_type.substr(4) == "24") {
            data_quality.minimum = MATTER_INT_24_MIN;
            data_quality.maximum = MATTER_INT_24_MAX;
        } else if (matter_type.substr(4) == "32") {
            data_quality.minimum = MATTER_INT_32_MIN;
            data_quality.maximum = MATTER_INT_32_MAX;
        } else if (matter_type.substr(4) == "40") {
            data_quality.minimum = MATTER_INT_40_MIN;
            data_quality.maximum = MATTER_INT_40_MAX;
        } else if (matter_type.substr(4) == "48") {
            data_quality.minimum = MATTER_INT_48_MIN;
            data_quality.maximum = MATTER_INT_48_MAX;
        } else if (matter_type.substr(4) == "56") {
            data_quality.minimum = MATTER_INT_56_MIN;
            data_quality.maximum = MATTER_INT_56_MAX;
        } else if (matter_type.substr(4) == "64") {
            data_quality.minimum = MATTER_INT_64_MIN;
            data_quality.maximum = MATTER_INT_64_MAX;
        }
    }

    // If the type is not a known standard type
    std::cout << "Found: " << matter_type << std::endl;
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

//! Matter Access Type -> SDF Mapping
//! This function is used standalone to move all qualities to the SDF Mapping
void MapMatterAccess(const matter::Access& access)
{
    auto* access_reference = new ReferenceTreeNode("access");
    current_node->AddChild(access_reference);
    if (access.read.has_value())
        access_reference->AddAttribute("read", access.read.value());
    if (access.write.has_value())
        access_reference->AddAttribute("write", access.write.value());
    if (access.fabric_scoped.has_value())
        access_reference->AddAttribute("fabricScoped", access.fabric_scoped.value());
    if (access.fabric_sensitive.has_value())
        access_reference->AddAttribute("fabricSensitive", access.fabric_sensitive.value());
    if (!access.read_privilege.empty())
        access_reference->AddAttribute("readPrivilege", access.read_privilege);
    if (!access.write_privilege.empty())
        access_reference->AddAttribute("writePrivilege", access.write_privilege);
    if (!access.invoke_privilege.empty())
        access_reference->AddAttribute("invokePrivilege", access.invoke_privilege);
    if (access.timed.has_value())
        access_reference->AddAttribute("times", access.timed.value());
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
    auto* event_reference = new ReferenceTreeNode(event.name);
    current_node->AddChild(event_reference);
    // Export the id to the mapping
    event_reference->AddAttribute("id", (uint64_t) event.id);
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
    auto* command_reference = new ReferenceTreeNode(client_command.name);
    current_node->AddChild(command_reference);
    // Export the id to the mapping
    command_reference->AddAttribute("id", (uint64_t) client_command.id);
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
    auto* attribute_reference = new ReferenceTreeNode(attribute.name);
    current_node->AddChild(attribute_reference);

    // Export the id to the mapping
    attribute_reference->AddAttribute("id", (uint64_t) attribute.id);
    sdf_property.label = attribute.name;

    if (attribute.conformance.has_value())
        MapMatterConformance(attribute.conformance.value());

    if (attribute.access.has_value())
        MapMatterAccess(attribute.access.value());

    sdf_property.description = attribute.summary;

    // Map the Matter type onto data qualities
    MapMatterType(attribute.type, sdf_property);
    if (attribute.quality.has_value())
        MapOtherQuality(attribute.quality.value(), sdf_property);

    // sdf_property.default_ = attribute.default_;

    return sdf_property;
}

sdf::SdfObject MapMatterCluster(const matter::Cluster& cluster)
{
    sdf::SdfObject sdf_object;
    auto* cluster_reference = new ReferenceTreeNode(cluster.name);
    current_node->AddChild(cluster_reference);

    cluster_reference->AddAttribute("id", (uint64_t) cluster.id);
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
    auto* sdf_action_node = new ReferenceTreeNode("sdfAction");
    cluster_reference->AddChild(sdf_action_node);
    current_node = sdf_action_node;
    for (const auto& command : cluster.commands){
        sdf::SdfAction sdf_action = MapMatterCommand(command);
        sdf_object.sdf_action.insert({command.name, sdf_action});
    }

    // Iterate through the events and map them
    auto* sdf_event_node = new ReferenceTreeNode("sdfEvent");
    cluster_reference->AddChild(sdf_event_node);
    current_node = sdf_event_node;
    for (const auto& event : cluster.events){
        sdf::SdfEvent sdf_event =  MapMatterEvent(event);
        sdf_object.sdf_event.insert({event.name, sdf_event});
    }

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

sdf::SdfThing MapMatterDevice(const matter::Device& device)
{
    sdf::SdfThing sdf_thing;
    // Append a new sdf_object node to the tree
    auto* device_reference = new ReferenceTreeNode(device.name);
    current_node->AddChild(device_reference);

    device_reference->AddAttribute("id", (uint64_t) device.id);
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

    sdf_mapping.map = reference_tree.GenerateMapping(reference_tree.root);
    std::cout << sdf_mapping.map.size() << std::endl;
    // Print the resulting tree
    //simple_walker walker;
    //referenceTree.traverse(walker);

    return 0;
}
