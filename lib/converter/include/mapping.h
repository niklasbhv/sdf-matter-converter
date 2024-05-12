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
 * Helper structures and functions to map matter and sdf.
 */

#ifndef MAPPING_H
#define MAPPING_H

#include "matter.h"
#include "sdf.h"

/**
 * @brief Map a sdf-model and sdf-mapping to a matter object.
 *
 * This function maps a sdf-model and sdf-mapping onto a matter device and cluster definition.
 *
 * @param sdfModel The input sdf-model.
 * @param sdfMappingType The input sdf-mapping.
 * @param device The resulting device definition.
 * @return 0 on success, negative on failure.
 */
int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType, deviceType& device);

/**
 * @brief Map a device and cluster definition to a sdf-object.
 *
 * This function maps a device onto a sdf-model and sdf-mapping.
 *
 * @param device The input device definition.
 * @param sdfModel The resulting sdf-model.
 * @param sdfMapping The resulting sdf-mapping.
 * @return 0 on success, negative on failure.
 */
int map_matter_to_sdf(deviceType& device, sdfModelType& sdfModel, sdfMappingType& sdfMapping);

int map_matter_attribute(attributeType& attribute, sdfPropertyType& sdfProperty, pugi::xml_node& sdf_property_node);

int map_matter_command(commandType& client_command, sdfActionType& sdfAction, pugi::xml_node& sdf_action_node);

int map_matter_event(eventType& event, sdfEventType& sdfEvent, pugi::xml_node& sdf_event_node);

int map_sdf_property(sdfPropertyType& sdfProperty, attributeType& attribute, pugi::xml_node& sdf_property_node);

int map_sdf_action(sdfActionType& sdfAction, commandType& command);

int map_sdf_event(sdfEventType& sdfEvent, eventType& event);

#endif //MAPPING_H
