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

struct accessType{
    //! Each access is a combination of [RW + FS + VOMA + T] seperated by spaces
    //! R -> Read
    //! W -> Write
    //! RW -> Read + Write
    bool read;
    bool write;
    //! Fabric
    //! F -> Fabric Scoped Quality
    //! S -> Fabric Sensitive Quality
    //! Privileges
    //! V -> Read or Invoke access requires view privilege
    //! O -> Read, Write or Invoke access requires operate privilege
    //! M -> Read, Write or Invoke access requires manage privilege
    //! A -> Read, Write or Invoke access requires administer privilege
    //! Timed
    //! T -> Write or Invoke Access with timed interaction only
};

/**
 * @brief Struct which contains Matter event information
 */
struct eventType {

};

/**
 * @brief Struct which contains Matter command information
 */
struct commandType {

};

/**
 * @brief Struct which contains Matter attribute information
 */
struct attributeType {

};

/**
 * @brief Struct which contains Matter cluster information.
 */
struct clusterType {

};

/**
 *  @brief Struct which contains Matter device information.
 */
struct deviceType {

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
int parseDevice(const pugi::xml_node& device_xml, const pugi::xml_node& cluster_xml, deviceType& device);


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
int serializeDevice(const deviceType& device, pugi::xml_node& device_xml, pugi::xml_node& cluster_xml);

#endif //MATTER_H
