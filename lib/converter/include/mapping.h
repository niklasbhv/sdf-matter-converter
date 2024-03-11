#ifndef MAPPING_H
#define MAPPING_H

#include "matter.h"
#include "sdf.h"

int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType);
int return_matter_to_sdf(deviceType& device, clusterType& cluster);

#endif //MAPPING_H
