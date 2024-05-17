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
 * Structs to contain parsed information for matter and functions to parse from xml files.
 */
#ifndef MATTER_H
#define MATTER_H

#include <string>
#include <map>
#include <list>
#include <pugixml.hpp>
#include <optional>
#include <variant>
#include "matter_constants.h"

typedef std::variant<unsigned int, int, double, std::string, bool> defaultType;

/**
 * Struct which represents the quality column.
 */
struct otherQualityType {
    //! X -> Nullable.
    std::optional<bool> nullable;
    //! N -> Non-Volatile.
    std::optional<bool> non_volatile;
    //! F -> Fixed.
    std::optional<bool> fixed;
    //! S -> Scene.
    std::optional<bool> scene;
    //! P -> Reportable.
    std::optional<bool> reportable;
    //! C -> Changes Omitted.
    std::optional<bool> changes_omitted;
    //! I -> Singleton.
    std::optional<bool> singleton;
    //! Any of the above can be negated by using !.
};

struct constraintType;

struct constraintType {
    std::string type;
    //! The interpretation for each of these values depends on the data type its applied to
    //! Exact value.
    std::optional<int> value;
    //! For range constraints -> x to y.
    std::optional<std::tuple<int, int>> range;
    //! Minimum value.
    std::optional<int> min;
    //! Maximum value.
    std::optional<int> max;
    //! No constraints.
    //! Same as min to max.
    std::optional<bool> all;
    //! In case of a Union of multiple constraints, these get "chained" to each other
    //std::optional<constraintType> constraint;
    //! Used for list_constraint[entry_constraint].
    //! List constraint is mapped to the above qualities, entry constraint is mapped to the below quality.
    std::optional<bool> entry_constraint;
    //! char_constraint[z].
    //! char_constraint is the string constraint in bytes.
    //! z is the maximum number of unicode codepoints.
    //! char_constraint gets mapped with the above qualities, z is mapped to the below quality.
    std::optional<int> max_unicode_endpoints;
};

/**
 * Type used to store revision information.
 * Maps a revision onto a summary of changes.
 */
typedef std::map<u_int8_t, std::string> revisionType;

/**
 * Struct which represents feature conformance.
 */
struct conformanceType {
    //! M -> Mandatory
    std::optional<bool> mandatory;
    //! O -> Optional
    std::optional<bool> optional;
    //! P -> Provisional
    std::optional<bool> provisional;
    //! D -> Deprecated
    std::optional<bool> deprecated;
    //! X -> Disallowed
    std::optional<bool> disallowed;
    //! Represents the boolean expression.
    std::optional<std::string> condition;
};

/**
 * Struct which represents access qualities.
 */
struct accessType {
    //! Each access is a combination of [RW FS VOMA T] seperated by spaces
    //! R -> Read
    //! W -> Write
    //! RW -> Read + Write
    std::optional<bool> read;
    std::optional<bool> write;
    //! Fabric
    //! F -> Fabric Scoped Quality
    //! S -> Fabric Sensitive Quality
    std::optional<bool> fabric_scoped;
    std::optional<bool> fabric_sensitive;
    //! Privileges
    //! V -> Read or Invoke access requires view privilege
    //! O -> Read, Write or Invoke access requires operate privilege
    //! M -> Read, Write or Invoke access requires manage privilege
    //! A -> Read, Write or Invoke access requires administer privilege
    std::optional<bool> requires_view_privilege;
    std::optional<bool> requires_operate_privilege;
    std::optional<bool> requires_manage_privilege;
    std::optional<bool> requires_administer_privilege;
    //! Timed
    //! T -> Write or Invoke Access with timed interaction only
    std::optional<bool> timed;
};

/**
 * Struct which represents the common data.
 */
struct commonDataQualityType {
    //! Unique identifier.
    u_int32_t id;
    //! CamelCase name of the element.
    std::string name;
    //! Field is being stripped as it is deprecated, use name instead.
    //! Defines dependencies.
    conformanceType conformance;
    //! Defines how an element is accessed.
    accessType access;
    //! Short summary of the element.
    std::string summary;
};

struct dataType : otherQualityType {
    std::string dataType;
    constraintType constraint;
    accessType access;
    std::string default_;
    conformanceType conformance;
};

// TODO: Temporary, derived from the xml definitions
struct enumItemType {
    int value;
    std::string name;
    std::string summary;
    conformanceType conformance;
};

// TODO: Temporary, derived from the xml definitions
struct bitmapBitfieldType {
    int bit;
    std::string name;
    std::string summary;
    conformanceType conformance;
};

// TODO: Temporary, derived from the xml definitions
struct structFieldType {
    u_int32_t id;
    std::string name;
    std::string type;
    conformanceType conformance;
};

/**
 * Struct which represents FeatureMap Attribute.
 * Used to define optional features.
 */
struct featureMapType {
    u_int8_t bit;
    conformanceType conformance;
    //! Capitalized, short code
    std::string code;
    std::string name;
    std::string summary;
};

struct eventFieldType {
    u_int32_t id;
    std::string name;
    std::string type;
    std::string default_;
    accessType access;
    conformanceType conformance;
    otherQualityType qualities;
    constraintType constraints;
};

/**
 * Struct which contains Matter event information.
 */
struct eventType : commonDataQualityType {
    //! 0 -> DEBUG, 1 -> INFO, 2 -> CRITICAL
    u_int8_t priority;
    u_int64_t number;
    //! Either system-time-ms or posix-ms
    u_int64_t timestamp;
    // data -> struct
};

/**
 * Struct which contains Matter command information.
 */
struct commandType : commonDataQualityType {
    std::string default_;
    //! clientToServer or serverToClient
    std::string direction;
    //! N, Y, or name of the response command
    std::string response; // Also indicates, whether the command is a response or request command
};

// TODO: Generics and lists dont seem, to be a good combination, keeping string for now
/**
 * Struct which contains Matter attribute information.
 */
struct attributeType : commonDataQualityType {
    std::string type;
    otherQualityType qualities;
    std::string default_;
};

/**
 * Struct which contains Matter cluster information.
 */
struct clusterType : commonDataQualityType{
    int revision;
    revisionType revision_history;
    // TODO: Change this into a enum
    std::string classification;
    std::list<attributeType> attributes;
    std::list<commandType> commands;
    std::list<eventType> events;
};

/**
 *  Struct which contains Matter device information.
 */
struct deviceType : commonDataQualityType{
    int revision;
    revisionType revision_history;
    // TODO: Change this into a enum
    std::string classification;
    std::list<clusterType> clusters;
    std::list<featureMapType> features;
    std::map<std::string, std::list<enumItemType>> enums;
    std::map<std::string, std::list<bitmapBitfieldType>> bitmaps;
    std::map<std::string, std::list<structFieldType>> structs;
};


/**
 * @brief Parses xml-file into a device.
 *
 * This function takes a device definition and the cluster definitions in xml format and converts it into a device.
 *
 * @param device_xml Device definitions in xml format.
 * @param cluster_xml Cluster definitions in xml format.
 * @param device The resulting device.
 * @return 0 on success, negative on failure.
 */
int parse_device(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device);


/**
 * @brief Serializes device into xml-file.
 *
 * This functions takes a device object and serializes it into a device and a cluster xml.
 *
 * @param device The input device object.
 * @param device_xml The resulting device xml.
 * @param cluster_xml The resulting cluster xml.
 * @return 0 on success, negative on failure.
 */
int serialize_device(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml);

#endif //MATTER_H
