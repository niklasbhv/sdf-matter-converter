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

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <argparse/argparse.hpp>
#include <converter.h>
#include "main.h"

#define VALIDATE

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sdf-matter-converter");

    program.add_argument("--convert-to-sdf")
        .help("Convert from Matter to SDF")
        .default_value(false);

    program.add_argument("--convert-to-matter")
        .help("Convert from SDF to Matter")
        .default_value(false);

    program.add_argument("--validate-sdf")
        .help("Validate the input SDF files against a schema")
        .default_value(false);

    program.add_argument("--validate-xml")
        .help("Validate the input XML files against a schema")
        .default_value(false);

    program.add_argument("-o", "-output")
        .required()
        .help("Specify the output file");

    program.add_argument("-p", "-path")
        .required()
        .help("Path containing the necessary files for the conversion");

    program.add_argument("-sdf-model")
        .help("Input JSON containing the SDF Model, required for conversion to Matter");

    program.add_argument("-sdf-mapping")
        .help("Input JSON containing the SDF Mapping, required for conversion to Matter");

    program.add_argument("-device-xml")
        .help("Input XML containing the Device Definitions, required for conversion to SDF");

    program.add_argument("-cluster-xml")
        .help("Input XML containing the Cluster Definitions, required for conversion to SDF");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if(program.is_used("--convert-to-sdf"))
    {
        if(!(program.is_used("-device-xml") and program.is_used("-cluster-xml")))
        {
            std::cerr << "Device XML or Cluster XML missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_device_xml = program.get<std::string>("-device-xml");
        pugi::xml_document device_xml;
        loadXmlFile(path_device_xml.c_str(), device_xml);
        auto path_cluster_xml = program.get<std::string>("-cluster-xml");
        pugi::xml_document cluster_xml;
        loadXmlFile(path_cluster_xml.c_str(), cluster_xml);
        convertMatterToSdf(device_xml, cluster_xml);
    }
    if(program.is_used("--convert-to-matter"))
    {
        if(!(program.is_used("-sdf-model") and program.is_used("-sdf-mapping")))
        {
            std::cerr << "SDF Model or SDF Mapping missing as an input argument" << std::endl;
            std::exit(1);
        }
        auto path_sdf_model = program.get<std::string>("-sdf-model");
        json sdf_model;
        loadJsonFile(path_sdf_model.c_str(), sdf_model);
        auto path_sdf_mapping = program.get<std::string>("-sdf-mapping");
        json sdf_mapping;
        loadJsonFile(path_sdf_mapping.c_str(), sdf_mapping);
        convertSdfToMatter(sdf_model, sdf_mapping);
    }

    if(program["--validate-sdf"] == true)
    {

    }

    return 0;
}
