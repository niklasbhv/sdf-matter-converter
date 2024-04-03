/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Functions validate sdf and matter against a schema.
 */

#ifndef VALIDATOR_H
#define VALIDATOR_H

/**
 * @brief Check compliance for sdf file against schema.
 *
 * This function checks, if a given file complies with the given schema.
 *
 * @param path Path to the file.
 * @param schema Path to the schema.
 * @return 0 on success, negative on failure.
 */
int validateSdf(const char* path, const char* schema);

/**
 * @brief Check compliance for matter file against schema.
 *
 * This function checks, if a given file complies with the given schema.
 *
 * @param path Path to the file.
 * @param schema Path to the schema.
 * @return 0 on success, negative on failure.
 */
int validateMatter(const char* path, const char* schema);

#endif //VALIDATOR_H
