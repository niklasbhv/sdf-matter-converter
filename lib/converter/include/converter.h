//
// Created by niklas on 28.02.24.
//

#ifndef CONVERTER_H
#define CONVERTER_H

#include <nlohmann/json.hpp>

int loadJsonFile(const char* path, nlohmann::json json_file);
int convertSdfToMatter(const nlohmann::json& sdf_model, const nlohmann::json& sdf_mapping);

#endif //CONVERTER_H
