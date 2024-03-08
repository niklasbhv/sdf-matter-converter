#ifndef SDF_MATTER_CONVERTER_MAPPING_H
#define SDF_MATTER_CONVERTER_MAPPING_H

#include "matter.h"
#include "sdf.h"

int map_sdf_to_matter(sdfModelType& sdfModel, sdfMappingType& sdfMappingType);
int return_matter_to_sdf(deviceType& device, clusterType& cluster);

#endif //SDF_MATTER_CONVERTER_MAPPING_H
