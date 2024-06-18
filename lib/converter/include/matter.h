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
#define MATTER_U_INT_8_MIN 0
#define MATTER_U_INT_8_MAX 255
#define MATTER_U_INT_16_MIN 0
#define MATTER_U_INT_16_MAX 65535
#define MATTER_U_INT_24_MIN 0
#define MATTER_U_INT_24_MAX 16777215
#define MATTER_U_INT_32_MIN 0
#define MATTER_U_INT_32_MAX 4294967295
#define MATTER_U_INT_40_MIN 0
#define MATTER_U_INT_40_MAX 1099511627775
#define MATTER_U_INT_48_MIN 0
#define MATTER_U_INT_48_MAX 281474976710655
#define MATTER_U_INT_56_MIN 0
#define MATTER_U_INT_56_MAX 72057594037927935
#define MATTER_U_INT_64_MIN 0
#define MATTER_U_INT_64_MAX ((uint64_t)18446744073709551615)

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
#define MATTER_INT_64_MIN ((int64_t)-9223372036854775808)
#define MATTER_INT_64_MAX ((int64_t)9223372036854775807)

/**
 * Function used to convert decimal uint32 numbers to hexadecimal.
 */
inline std::string decToHexa(uint32_t n)
{
    // TODO: Has to be reworked in order to account für uint32 size numbers
    // ans string to store hexadecimal number
    std::string ans = "";

    while (n != 0) {
        // remainder variable to store remainder
        int rem = 0;

        // ch variable to store each character
        char ch;
        // storing remainder in rem variable.
        rem = n % 16;

        // check if temp < 10
        if (rem < 10) {
            ch = rem + 48;
        }
        else {
            ch = rem + 55;
        }

        // updating the ans string with the character variable
        ans += ch;
        n = n / 16;
    }

    // reversing the ans string to get the final result
    while (ans.length() < 4) {
        ans.append("0");
    }
    ans.append("x0"); // This will get reversed
    int i = 0, j = ans.size() - 1;
    while(i <= j)
    {
        std::swap(ans[i], ans[j]);
        i++;
        j--;
    }

    return ans;
}

namespace matter {
/**
 * Type definition for the default type.
 */
typedef std::variant<uint64_t, int64_t, double, std::string> defaultType;
typedef std::variant<double, int64_t, uint64_t> numericType;
/**
 * Type used to store revision information.
 * Maps a revision id onto a summary of changes.
 */
typedef std::map<u_int8_t, std::string> revisionType;

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
struct constraintType;

/**
 * Struct which represents constraints.
 */
struct constraintType {
    std::string type;
    //! The interpretation for each of these values depends on the data type its applied to
    //! Exact value.
    std::optional<defaultType> value;
    //! For range constraints -> x to y.
    std::optional<numericType> from;
    std::optional<numericType> to;
    //! Minimum value.
    std::optional<numericType> min;
    //! Maximum value.
    std::optional<numericType> max;
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
struct conformanceType {
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
    std::list<conformanceType> otherwise;
    //! List representing the choice element
    std::list<conformanceType> choice;
    //! Represents the entire logical term
    //! Note that this will be in a C++ fashion format
    std::string condition;
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
struct commonDataQualityType {
    //! Unique identifier.
    u_int32_t id;
    //! CamelCase name of the element.
    std::string name;
    //! Field is being stripped as it is deprecated, use name instead.
    //! Defines dependencies.
    std::optional<conformanceType> conformance;
    //! Defines how an element is accessed.
    std::optional<accessType> access;
    //! Short summary of the element.
    std::string summary;
};

// TODO: Temporary, derived from the xml definitions
struct enumItemType {
    int value;
    std::string name;
    std::string summary;
    std::optional<conformanceType> conformance;
};

// TODO: Temporary, derived from the xml definitions
struct bitmapBitfieldType {
    int bit;
    std::string name;
    std::string summary;
    std::optional<conformanceType> conformance;
};

/**
 * Struct which contains data field information.
 */
struct dataFieldType : commonDataQualityType {
    //! Data type
    std::string type;
    //! Constraints
    std::optional<constraintType> constraint;
    //! Other qualities
    std::optional<otherQualityType> quality;
    //! Default value
    defaultType default_;
};

/**
 * Type definition for the struct.
 */
typedef std::list<dataFieldType> structType;

/**
 * Struct which represents FeatureMap Attribute.
 * Used to define optional features.
 */
struct featureMapType {
    u_int8_t bit;
    std::optional<conformanceType> conformance;
    //! Capitalized, short code
    std::string code;
    std::string name;
    std::string summary;
};

/**
 * Struct which contains Matter event information.
 */
struct eventType : commonDataQualityType {
    //! Currently either debug, info or critical
    std::string priority;
    //! Other qualities
    std::optional<otherQualityType> quality;
    //! Data used for the event record
    structType data;
};

/**
 * Struct which contains Matter command information.
 */
struct commandType : commonDataQualityType {
    defaultType default_;
    //! Either commandToServer or responseFromServer
    std::string direction;
    //! Either Y, N or the name of the response command
    std::string response;
    //! Command fields
    structType command_fields;
};

/**
 * Struct which contains Matter attribute information.
 */
struct attributeType : commonDataQualityType {
    //! Data type
    std::string type;
    //! Other qualities
    std::optional<otherQualityType> quality;
    //! Default value
    defaultType default_;
};

/**
 * Struct which contains cluster classification information.
 */
struct clusterClassificationType {
    //! Either base or derived
    std::string hierarchy;
    //! Either utility or application
    std::string role;
    //! Upper case identification code
    std::string picsCode;
    //! Either Endpoint or Node
    std::string scope;
    //! Cluster name of the base cluster
    std::string baseCluster;
    //! Number of the primary transaction
    std::string primaryTransaction;
};

/**
 * Struct which contains Matter cluster information.
 */
struct clusterType : commonDataQualityType {
    //! Current revision
    int revision;
    //! History of revisions
    revisionType revision_history;
    //! Cluster classification
    std::optional<clusterClassificationType> classification;
    //! List of attributes
    std::list<attributeType> attributes;
    //! List of commands
    std::list<commandType> commands;
    //! List of events
    std::list<eventType> events;
    //! Map for globally defined enums
    std::map<std::string, std::list<enumItemType>> enums;
    //! Map for globally defined bitmaps
    std::map<std::string, std::list<bitmapBitfieldType>> bitmaps;
};

/**
 * Struct which contains device classification information.
 */
struct deviceClassificationType {
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
struct deviceType : commonDataQualityType {
    //! Current revision
    int revision;
    //! History of revisions
    revisionType revision_history;
    //! Device classification
    std::optional<deviceClassificationType> classification;
    //! Device conditions
    std::map<std::string, std::string> conditions;
    //! List of used clusters
    std::list<clusterType> clusters;
};
/**
 * @brief Parses xml-file into a device.
 *
 * This function takes a device type definition and the cluster definitions in XML format and converts it into a device.
 *
 * @param device_xml Device definitions in xml format.
 * @param cluster_xml Cluster definitions in xml format.
 * @param device The resulting device.
 * @param client If the resulting SDF-Model should be a client.
 * @return 0 on success, negative on failure.
 */
int parse_device(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device, bool client);


/**
 * @brief Parses xml-file into a cluster.
 *
 * This functions takes a cluster definition in XML format and converts it into a cluster.
 *
 * @param cluster_xml Cluster definition in XML format.
 * @param cluster The resulting cluster.
 * @return 0 on success, negative on failure.
 */
int parse_cluster(const pugi::xml_node& cluster_xml, clusterType& cluster);


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

int serialize_cluster(const clusterType& cluster, pugi::xml_node& cluster_xml);

} // namespace matter

#endif //MATTER_H
