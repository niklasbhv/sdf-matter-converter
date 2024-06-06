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
#define MATTER_U_INT_64_MAX 18446744073709551615

//! Max and Min Type boundaries if value is not nullable
//! For nullable values, min has to be increased by one
#define MATTER_INT_8_MIN -128
#define MATTER_INT_8_MAX 127
#define MATTER_INT_16_MIN -32768
#define MATTER_INT_16_MAX 32767
#define MATTER_INT_24_MIN -8388608
#define MATTER_INT_24_MAX 8388607
#define MATTER_INT_32_MIN -2147483648
#define MATTER_INT_32_MAX 2147483647
#define MATTER_INT_40_MIN -549755813888
#define MATTER_INT_40_MAX 549755813887
#define MATTER_INT_48_MIN -140737488355328
#define MATTER_INT_48_MAX 140737488355327
#define MATTER_INT_56_MIN -36028797018963968
#define MATTER_INT_56_MAX 36028797018963967
#define MATTER_INT_64_MIN -9223372036854775808
#define MATTER_INT_64_MAX 9223372036854775807

// function to convert decimal to hexadecimal
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

struct dataType : otherQualityType {
    std::string dataType;
    constraintType constraint;
    std::optional<accessType> access;
    std::string default_;
    std::optional<conformanceType> conformance;
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

struct structFieldType : commonDataQualityType{
    dataType data;
};

typedef std::list<structFieldType> structType;

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

struct eventRecordType {
    u_int64_t number;
    std::string timestamp;
    u_int8_t priority;
    structType data;
};

/**
 * Struct which contains Matter event information.
 */
struct eventType : commonDataQualityType {
    std::string priority;
    std::optional<otherQualityType> quality;
    std::list<eventRecordType> event_records;
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
    std::optional<otherQualityType> quality;
    std::string default_;
};

struct classificationType {
    std::string hierarchy;
    std::string role;
    std::string picsCode;
    std::string scope;
    std::string baseCluster;
    std::string primaryTransaction;
};

/**
 * Struct which contains Matter cluster information.
 */
struct clusterType : commonDataQualityType {
    int revision;
    revisionType revision_history;
    std::optional<classificationType> classification;
    std::list<attributeType> attributes;
    std::list<commandType> commands;
    std::list<eventType> events;
    std::map<std::string, std::list<enumItemType>> enums;
    std::map<std::string, std::list<bitmapBitfieldType>> bitmaps;
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
