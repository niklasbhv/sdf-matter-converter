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
 * Functions validate sdf and matter against a schema.
 */

#ifndef SDF_MATTER_CONVERTER_LIB_VALIDATOR_INCLUDE_VALIDATOR_H_
#define SDF_MATTER_CONVERTER_LIB_VALIDATOR_INCLUDE_VALIDATOR_H_

//! @brief Check compliance for sdf file against schema_path.
//!
//! This function checks, if a given file complies with the given schema_path.
//!
//! @param path Path to the file.
//! @param schema_path Path to the schema_path.
//! @return 0 on success, negative on failure.
int ValidateSdf(const char* path, const char* schema_path);

//! @brief Check compliance for matter file against schema_file.
//!
//! This function checks, if a given file complies with the given schema_file.
//!
//! @param xml_file Path to the file.
//! @param schema_file Path to the schema_file.
//! @return 0 on success, negative on failure.
int ValidateMatter(const char* path, const char* schema_path);

#endif //SDF_MATTER_CONVERTER_LIB_VALIDATOR_INCLUDE_VALIDATOR_H_
