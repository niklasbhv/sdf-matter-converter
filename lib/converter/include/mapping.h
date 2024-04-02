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
 * Stores the tree structure used for sdf-mapping.
 */
inline pugi::xml_document mappingTree;

/**
 * Stores the current node of the mapping tree
 */
inline pugi::xml_node mappingTreeCurrentNode = mappingTree.root();

/**
 * @brief Map a sdf-model and sdf-mapping to a matter object.
 *
 * This function maps a sdf-model and sdf-mapping onto a matter device and cluster definition.
 *
 * @param sdfModel The input sdf-model.
 * @param sdfMappingType The input sdf-mapping.
 * @return 0 on success, negative on failure.
 */
int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType);

/**
 * @brief Map a device and cluster definition to a sdf-object.
 *
 * This function maps a device and list of cluster definitions onto a sdf-model and sdf-mapping.
 *
 * @param device The input device definition.
 * @param clusterList The input cluster definitions.
 * @return 0 on success, negative on failure.
 */
int map_matter_to_sdf(deviceType& device, std::list<clusterType>& clusterList);

#endif //MAPPING_H
