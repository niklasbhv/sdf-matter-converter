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

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sdf-matter-converter");

    program.add_argument("--matter-to-sdf")
        .help("Convert from Matter to SDF")
        .default_value(false);

    program.add_argument("--sdf-to-matter")
        .help("Convert from SDF to Matter")
        .default_value(false);

    program.add_argument("--validate")
        .help("Validate the input and output files")
        .default_value(false);

    program.add_argument("--round-trip")
        .help("Use round-tripping to convert from one format to the other and back to the original format")
        .default_value(false);

    program.add_argument("-sdf-model")
        .help("Input JSON containing the SDF Model, required for conversion to Matter");

    program.add_argument("-sdf-mapping")
        .help("Input JSON containing the SDF Mapping, required for conversion to Matter");

    program.add_argument("-device-xml")
        .help("Input XML containing the Device Definitions, required for conversion to SDF");

    program.add_argument("-cluster-xml")
        .help("Input XML containing the Cluster Definitions, required for conversion to SDF");

    program.add_argument("-o", "-output")
        .help("Specify the output file");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if(program.is_used("--matter-to-sdf"))
    {
        if(!(program.is_used("-device-xml") and program.is_used("-cluster-xml")))
        {
            std::cerr << "Device XML or Cluster XML missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_device_xml = program.get<std::string>("-device-xml");
        auto path_cluster_xml = program.get<std::string>("-cluster-xml");

        if (program.get<bool>("--validate")){
            std::cout << "Validating input XML files..." << std::endl;
            //TODO: Schema is empty for now
            if (validateMatter(path_device_xml.c_str(), "") == 0)
                std::cout << "Device XML is valid!" << std::endl;
            else
                std::cout << "Device XML is not valid!" << std::endl;
            if (validateMatter(path_cluster_xml.c_str(), "") == 0)
                std::cout << "Cluster XML is valid!" << std::endl;
            else
                std::cout << "Cluster XML is not valid!" << std::endl;
        }

        std::cout << "Loading XML files..." << std::endl;
        pugi::xml_document device_xml;
        loadXmlFile(path_device_xml.c_str(), device_xml);
        std::cout << "Successfully loaded Device XML!" << std::endl;

        pugi::xml_document cluster_xml;
        loadXmlFile(path_cluster_xml.c_str(), cluster_xml);
        std::cout << "Successfully loaded Cluster XML!" << std::endl;

        std::cout << "Converting Matter to SDF..." << std::endl;
        convertMatterToSdf(device_xml, cluster_xml);
        std::cout << "Successfully converted Matter to SDF!" << std::endl;

        std::cout << "Saving JSON files...." << std::endl;
        //saveJsonFile();
        std::cout << "Successfully saved SDF-Model!" << std::endl;

        //saveJsonFile();
        std::cout << "Successfully saved SDF-Mapping!" << std::endl;

        if (program.get<bool>("--validate")){
            std::cout << "Validating output JSON files..." << std::endl;
            //TODO: Input and Schema are empty for now
            if (validateSdf("", "") == 0)
                std::cout << "SDF-Model JSON is valid!" << std::endl;
            else
                std::cout << "SDF-Model JSON is not valid!" << std::endl;
            if (validateSdf("", "") == 0)
                std::cout << "SDF-Mapping JSON is valid!" << std::endl;
            else
                std::cout << "SDF-Mapping JSON is not valid!" << std::endl;
        }
    }
    else if(program.is_used("--sdf-to-matter"))
    {
        if(!(program.is_used("-sdf-model") and program.is_used("-sdf-mapping")))
        {
            std::cerr << "SDF Model or SDF Mapping missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_sdf_model = program.get<std::string>("-sdf-model");
        auto path_sdf_mapping = program.get<std::string>("-sdf-mapping");

        if (program.get<bool>("--validate")){
            std::cout << "Validating input JSON files..." << std::endl;
            //TODO: Schema is empty for now
            if (validateSdf("", "") == 0)
                std::cout << "SDF-Model JSON is valid!" << std::endl;
            else
                std::cout << "SDF-Model JSON is not valid!" << std::endl;
            if (validateSdf("", "") == 0)
                std::cout << "SDF-Mapping JSON is valid!" << std::endl;
            else
                std::cout << "SDF-Mapping JSON is not valid!" << std::endl;
        }

        std::cout << "Loading JSON files..." << std::endl;
        json sdf_model;
        loadJsonFile(path_sdf_model.c_str(), sdf_model);
        std::cout << "Successfully loaded SDF-Model JSON!" << std::endl;

        json sdf_mapping;
        loadJsonFile(path_sdf_mapping.c_str(), sdf_mapping);
        std::cout << "Successfully loaded SDF-Mapping JSON!" << std::endl;

        std::cout << "Converting SDF to Matter..." << std::endl;
        convertSdfToMatter(sdf_model, sdf_mapping);
        std::cout << "Successfully converted SDF to Matter!" << std::endl;

        std::cout << "Saving XML files...." << std::endl;
        //saveXmlFile();
        std::cout << "Successfully saved Device XML!" << std::endl;

        //saveXmlFile();
        std::cout << "Successfully saved Cluster XML!" << std::endl;

        if (program.get<bool>("--validate")){
            std::cout << "Validating output XML files..." << std::endl;
            //TODO: Input and Schema are empty for now
            if (validateMatter("", "") == 0)
                std::cout << "Device XML is valid!" << std::endl;
            else
                std::cout << "Device XML is not valid!" << std::endl;
            if (validateMatter("", "") == 0)
                std::cout << "Cluster XML is valid!" << std::endl;
            else
                std::cout << "Cluster XML is not valid!" << std::endl;
        }
    }
    // Print help of neither convert-to-sdf nor convert-to-matter are given
    else{
        std::cout << program;
    }

    return 0;
}
