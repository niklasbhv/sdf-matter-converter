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

//! Max and Min Type boundaries if value is not nullable
//! For nullable values, max has to be decreased by one
#define MATTER_U_INT_8_MAX 255
#define MATTER_U_INT_16_MAX 65535
#define MATTER_U_INT_24_MAX 16777215
#define MATTER_U_INT_32_MAX 4294967295
#define MATTER_U_INT_40_MAX 1099511627775
#define MATTER_U_INT_48_MAX 281474976710655
#define MATTER_U_INT_56_MAX 72057594037927935

//! Max and Min Type boundaries if value is not nullable
//! For nullable values, min has to be increased by one
#define MATTER_INT_8_MIN (-128)
#define MATTER_INT_8_MAX 127
#define MATTER_INT_16_MIN (-32768)
#define MATTER_INT_16_MAX 32767
#define MATTER_INT_24_MIN (-8388608)
#define MATTER_INT_24_MAX 8388607
#define MATTER_INT_32_MIN (-2147483648)
#define MATTER_INT_32_MAX 2147483647
#define MATTER_INT_40_MIN (-549755813888)
#define MATTER_INT_40_MAX 549755813887
#define MATTER_INT_48_MIN (-140737488355328)
#define MATTER_INT_48_MAX 140737488355327
#define MATTER_INT_56_MIN (-36028797018963968)
#define MATTER_INT_56_MAX 36028797018963967

// TODO: Check if this is neccessary for ids
inline u_int32_t HexToInt(const std::string& hexStr) {
    int result = 0;
    for (char ch : hexStr) {
        result *= 16;
        if (ch >= '0' && ch <= '9') {
            result += ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            result += ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'F') {
            result += ch - 'A' + 10;
        } else {
            throw std::invalid_argument("Invalid hex character");
        }
    }
    return result;
}

inline std::string IntToHex(u_int32_t num) {
    std::string hex_str;
    while (num > 0) {
        int remainder = num % 16;
        if (remainder < 10) {
            hex_str = static_cast<char>('0' + remainder) + hex_str;
        } else {
            hex_str = static_cast<char>('A' + (remainder - 10)) + hex_str;
        }
        num /= 16;
    }
    while (hex_str.length() < 4) {
        hex_str.insert(0, 1, '0');
    }
    hex_str.insert(0, "0x");

    return hex_str;
}

namespace matter {

//!Type definition for the default type.
typedef std::variant<double, int64_t, uint64_t, std::string> DefaultType;

//! Type definition for the numeric type.
typedef std::variant<double, int64_t, uint64_t> NumericType;

//!Type used to store revision information.
//!Maps a revision id onto a summary of changes.
typedef std::map<u_int8_t, std::string> Revision;

/**
 * Struct which represents the quality column.
 */
struct OtherQuality {
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
    //! C -> Change Omitted.
    std::optional<bool> change_omitted;
    //! I -> Singleton.
    std::optional<bool> singleton;
    //! K -> Diagnostics.
    std::optional<bool> diagnostics;
    //! L -> Large Message
    std::optional<bool> large_message;
    //! Q -> Quieter Reporting
    std::optional<bool> quieter_reporting;
    //! Any of the above can be negated by using !.
};

/**
 * Struct definition for constraint type.
 */
struct Constraint;

/**
 * Struct which represents constraints.
 */
struct Constraint {
    std::string type;
    //! The interpretation for each of these values depends on the data type its applied to
    //! Exact value.
    std::optional<DefaultType> value;
    //! For range constraints -> x to y.
    std::optional<NumericType> from;
    std::optional<NumericType> to;
    //! Minimum value.
    std::optional<NumericType> min;
    //! Maximum value.
    std::optional<NumericType> max;
    //! No constraints.
    //! Same as min to max.
    std::optional<bool> all;
    //! In case of a Union of multiple constraints, these get "chained" to each other
    struct contraintType *constraint;
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
 * Struct which represents feature conformance.
 */
struct Conformance {
    //! M -> Mandatory conformance
    std::optional<bool> mandatory;
    //! O -> Optional conformance
    std::optional<bool> optional;
    //! P -> Provisional conformance
    std::optional<bool> provisional;
    //! D -> Deprecated conformance
    std::optional<bool> deprecated;
    //! X -> Disallowed conformance
    std::optional<bool> disallowed;
    //! List representing the otherwise conformance
    //! Note that the first true conformance in this list will be chosen
    std::list<Conformance> otherwise;
    //! List representing the choice element
    std::list<Conformance> choice;
    //! Represents the entire logical term
    //! Note that this will be in a C++ fashion format
    std::string condition;
};

/**
 * Struct which represents access qualities.
 */
struct Access {
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
    std::string read_privilege; // view | operate | manage | administer
    std::string write_privilege; // operate | manage | administer
    std::string invoke_privilege; // view | operate | manage | administer
    //! Timed
    //! T -> Write or Invoke Access with timed interaction only
    std::optional<bool> timed;
};

/**
 * Struct which represents the common data.
 */
struct CommonQuality {
    //! Unique identifier.
    u_int32_t id;
    //! CamelCase name of the element.
    std::string name;
    //! Field is being stripped as it is deprecated, use name instead.
    //! Defines dependencies.
    std::optional<Conformance> conformance;
    //! Defines how an element is accessed.
    std::optional<Access> access;
    //! Short summary of the element.
    std::string summary;
};

// TODO: Temporary, derived from the xml definitions
struct Item {
    int value;
    std::string name;
    std::string summary;
    std::optional<Conformance> conformance;
};

// TODO: Temporary, derived from the xml definitions
struct Bitfield {
    int bit;
    std::string name;
    std::string summary;
    std::optional<Conformance> conformance;
};

/**
 * Struct which contains data field information.
 */
struct DataField : CommonQuality {
    //! Data type
    std::string type;
    //! Constraints
    std::optional<Constraint> constraint;
    //! Other qualities
    std::optional<OtherQuality> quality;
    //! Default value
    DefaultType default_;
};

//! Type definition for the struct.
typedef std::list<DataField> Struct;

/**
 * Struct which represents FeatureMap Attribute.
 * Used to define optional features.
 */
struct Feature {
    u_int8_t bit;
    std::optional<Conformance> conformance;
    //! Capitalized, short code
    std::string code;
    std::string name;
    std::string summary;
};

/**
 * Struct which contains Matter event information.
 */
struct Event : CommonQuality {
    //! Currently either debug, info or critical
    std::string priority;
    //! Other qualities
    std::optional<OtherQuality> quality;
    //! Data used for the event record
    Struct data;
};

/**
 * Struct which contains Matter command information.
 */
struct Command : CommonQuality {
    DefaultType default_;
    //! Either commandToServer or responseFromServer
    std::string direction;
    //! Either Y, N or the name of the response command
    std::string response;
    //! Command fields
    Struct command_fields;
};

/**
 * Struct which contains Matter attribute information.
 */
struct Attribute : CommonQuality {
    //! Data type
    std::string type;
    //! Constraints
    std::optional<Constraint> constraint;
    //! Other qualities
    std::optional<OtherQuality> quality;
    //! Default value
    DefaultType default_;
};

/**
 * Struct which contains cluster classification information.
 */
struct ClusterClassification {
    //! Either base or derived
    std::string hierarchy;
    //! Either utility or application
    std::string role;
    //! Upper case identification code
    std::string picsCode;
    //! Either Endpoint or Node
    std::string scope;
    //! Cluster name of the base cluster
    std::string base_cluster;
    //! Number of the primary transaction
    std::string primary_transaction;
};

/**
 * Struct which contains Matter cluster information.
 */
struct Cluster : CommonQuality {
    //! Current revision
    int revision;
    //! History of revisions
    Revision revision_history;
    //! Cluster classification
    std::optional<ClusterClassification> classification;
    //! Feature map
    std::list<Feature> feature_map;
    //! List of attributes
    std::list<Attribute> attributes;
    //! List of client commands
    std::list<Command> client_commands;
    //! Map of command names to their respective commands
    //! Improves the searching for server commands when matching them to their client commands
    std::map<std::string, Command> server_commands;
    //! List of events
    std::list<Event> events;
    //! Map for globally defined enums
    std::map<std::string, std::list<Item>> enums;
    //! Map for globally defined bitmaps
    std::map<std::string, std::list<Bitfield>> bitmaps;
};

/**
 * Struct which contains device classification information.
 */
struct DeviceClassification {
    //! Name of the superset device type
    std::string superset;
    //! Either simple, utility or node
    std::string class_;
    //! Either node or endpoint
    std::string scope;
};

/**
 *  Struct which contains Matter device type information.
 */
struct Device : CommonQuality {
    //! Current revision
    int revision;
    //! History of revisions
    Revision revision_history;
    //! Device classification
    std::optional<DeviceClassification> classification;
    //! Device conditions
    std::map<std::string, std::string> conditions;
    //! List of used clusters
    std::list<Cluster> clusters;
};

/**
 * @brief Parses xml-file into a device.
 *
 * This function takes a device type definition and the cluster definitions in XML format and converts it into a device.
 *
 * @param device_xml Device definitions in xml format.
 * @param client If the resulting SDF-Model should be a client.
 * @return The resulting device.
 */
Device ParseDevice(const pugi::xml_node& device_xml, bool client);


/**
 * @brief Parses xml-file into a cluster.
 *
 * This functions takes a cluster definition in XML format and converts it into a cluster.
 *
 * @param cluster_xml Cluster definition in XML format.
 * @return The resulting cluster.
 */
Cluster ParseCluster(const pugi::xml_node& cluster_xml);


/**
 * @brief Serializes device into xml-file.
 *
 * This functions takes a device object and serializes it into a device xml.
 *
 * @param device The input device object.
 * @return The resulting device xml.
 */
pugi::xml_document SerializeDevice(const Device& device);


/**
 * @brief Serializes cluster into xml-file.
 *
 * This functions takes a cluster object and serializes it into a cluster xml.
 *
 * @param device The input cluster object.
 * @return The resulting cluster xml.
 */
pugi::xml_document SerializeCluster(const Cluster& cluster);

} // namespace matter

#endif //MATTER_H
