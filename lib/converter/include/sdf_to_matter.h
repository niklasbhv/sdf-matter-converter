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
 * Helper structures and functions to map sdf to matter.
 */

#ifndef SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_SDF_TO_MATTER_H_
#define SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_SDF_TO_MATTER_H_

#include <optional>
#include <list>
#include "matter.h"
#include "sdf.h"

//! @brief Map a sdf-model and sdf-mapping to a matter object.
//!
//! This function maps a sdf-model and sdf-mapping onto a matter device and a list of clusters.
//!
//! @param sdf_model The input sdf-model.
//! @param sdf_mapping The input sdf-mapping.
//! @param device The optionally resulting device definition.
//! @param cluster_list The resulting list of cluster definitions.
//! @return 0 on success, negative on failure.
int MapSdfToMatter(const sdf::SdfModel& sdf_model,
                   const sdf::SdfMapping& sdf_mapping,
                   std::optional<matter::Device>& optional_device, std::list<matter::Cluster>& cluster_list);

#endif //SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_SDF_TO_MATTER_H_
