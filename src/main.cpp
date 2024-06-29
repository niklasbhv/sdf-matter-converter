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
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <argparse/argparse.hpp>
#include <converter.h>
#include "main.h"

using json = nlohmann::ordered_json;
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

// Helper function that generates sdf-model and sdf-mapping filenames
// Generates filenames of the format "path/to/file[-model|-mapping].json"
void GenerateSdfFilenames(const std::string& input, std::string& sdf_model_name, std::string& sdf_mapping_name){
    auto last_dot = input.find_last_of('.');

    sdf_model_name.append(input.substr(0, last_dot));
    sdf_model_name.append("-model");
    sdf_model_name.append(input.substr(last_dot));

    sdf_mapping_name.append(input.substr(0, last_dot));
    sdf_mapping_name.append("-mapping");
    sdf_mapping_name.append(input.substr(last_dot));
}

// Helper function that generates device and cluster filenames
// Generates filenames of the format "path/to/file[-device|-cluster].xml"
void GenerateMatterFilenames(const std::string& input, std::string& device_xml_name, std::string& cluster_xml_name){
    auto last_dot = input.find_last_of('.');

    device_xml_name.append(input.substr(0, last_dot));
    device_xml_name.append("-device");
    device_xml_name.append(input.substr(last_dot));

    cluster_xml_name.append(input.substr(0, last_dot));
    cluster_xml_name.append("-cluster");
    cluster_xml_name.append(input.substr(last_dot));
}


int sdf_to_matter(std::string& sdf_model_path,
                  std::string& sdf_mapping_path,
                  std::string& device_path,
                  std::string& cluster_path)
{
    std::cout << "Loading SDF-Model..." << std::endl;
    json sdf_model_json;
    LoadJsonFile(sdf_model_path.c_str(), sdf_model_json);

    std::cout << "Loading SDF-Mapping..." << std::endl;
    json sdf_mapping_json;
    LoadJsonFile(sdf_mapping_path.c_str(), sdf_mapping_json);

    std::cout << "Converting SDF to Matter" << std::endl;
    std::list<pugi::xml_document> cluster_xml_list;
    std::optional<pugi::xml_document> device_xml_optional;
    ConvertSdfToMatter(sdf_model_json, sdf_mapping_json, device_xml_optional, cluster_xml_list);

    if (device_xml_optional.has_value()) {
        std::cout << "Saving Device XML..." << std::endl;
        SaveXmlFile(device_path.c_str(), device_xml_optional.value());
    }

    std::cout << "Saving Cluster XML..." << std::endl;
    for (const auto& cluster_xml : cluster_xml_list) {
        SaveXmlFile(cluster_path.c_str(), cluster_xml);
    }

    return 0;
}

int matter_to_sdf(const std::optional<std::string>& device_xml_path,
                  std::string& cluster_folder_path,
                  std::string& sdf_model_path,
                  std::string& sdf_mapping_path)
{
    std::list<pugi::xml_document> cluster_xml_list;
    // Check if the given path points onto a folder or a file
    json sdf_model;
    json sdf_mapping;
    if (std::filesystem::is_directory(cluster_folder_path)) {
        std::cout << "Loading and Parsing every Cluster XML of the given path" << std::endl;
        for (const auto& dir_entry : recursive_directory_iterator(cluster_folder_path)) {
            pugi::xml_document cluster_xml;
            LoadXmlFile(dir_entry.path().c_str(), cluster_xml);
            cluster_xml_list.push_back(std::move(cluster_xml));
        }
    } else {
        std::cout << "Loading Cluster XML" << std::endl;
        pugi::xml_document cluster_xml;
        LoadXmlFile(cluster_folder_path.c_str(), cluster_xml);
        cluster_xml_list.push_back(std::move(cluster_xml));
    }
    if (device_xml_path.has_value()) {
        std::cout << "Loading Device XML" << std::endl;
        pugi::xml_document device_xml;
        LoadXmlFile(device_xml_path.value().c_str(), device_xml);
        std::cout << "Converting Matter to SDF" << std::endl;
        ConvertMatterToSdf(std::move(device_xml), cluster_xml_list, sdf_model, sdf_mapping);
    } else {
        std::cout << "Converting Matter to SDF" << std::endl;
        ConvertMatterToSdf(std::nullopt, cluster_xml_list, sdf_model, sdf_mapping);
    }

    SaveJsonFile(sdf_model_path.c_str(), sdf_model);
    SaveJsonFile(sdf_mapping_path.c_str(), sdf_mapping);

    return 0;
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sdf-matter-converter");

    program.add_argument("--matter-to-sdf")
        .help("Convert from Matter to SDF")
        .default_value(false);

    program.add_argument("--sdf-to-matter")
        .help("Convert from SDF to Matter")
        .default_value(false);

    program.add_argument("--round-trip")
        .help("Use round-tripping to convert from one format to the other and back to the original format")
        .default_value(false);

    program.add_argument("-sdf-model")
        .help("Path to the input JSON containing the SDF Model, required for conversion to Matter");

    program.add_argument("-sdf-mapping")
        .help("Path tp the input JSON containing the SDF Mapping, required for conversion to Matter");

    program.add_argument("-device-xml")
        .help("Path to a input XML containing the Device Type definition\n"
              "Requires specified clusters to be inside the given cluster folder");

    program.add_argument("-cluster-xml")
            .help("Path to a input XML containing a Cluster definition\n"
                  "Used without a Device Type definition to create a Model with a single sdf_object");

    program.add_argument("-validate")
        .help("Validate the output files\n"
              "Requires the path to the schema for the output files as an input");

    program.add_argument("-o", "-output")
        .required()
        .help("Specify the output file\n"
              "For the Matter to SDF conversion, this will get split up into -model and -mapping\n"
              "For the SDF to Matter conversion, this will get split up into -device and -clusters");

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
        if (program.is_used("-device-xml") and program.is_used("-cluster-xml")) {
            auto path_device_xml = program.get<std::string>("-device-xml");
            auto path_cluster_xml = program.get<std::string>("-cluster-xml");
            std::string sdf_model_path = "./converted-model.json";
            std::string sdf_mapping_path = "./converted-mapping.json";
            matter_to_sdf(path_device_xml, path_cluster_xml, sdf_model_path, sdf_mapping_path);

        }
        else if (program.is_used("-cluster-xml")) {
            auto path_cluster_xml = program.get<std::string>("-cluster-xml");
            std::string sdf_model_path = "./converted-model.json";
            std::string sdf_mapping_path = "./converted-mapping.json";
            matter_to_sdf(std::nullopt, path_cluster_xml, sdf_model_path, sdf_mapping_path);

            if (program.is_used("--round-trip")){
                std::cout << "Round-tripping flag was set!" << std::endl;
                std::cout << "Converting SDF to Matter..." << std::endl;
                pugi::xml_document round_trip_device_xml;
                pugi::xml_document round_trip_cluster_xml;
                //ConvertSdfToMatter(sdf_model, sdf_mapping,  round_trip_device_xml, round_trip_cluster_xml);
                std::cout << "Successfully converted SDF to Matter!" << std::endl;

                std::string path_round_trip_device_xml;
                std::string path_round_trip_cluster_xml;
                GenerateMatterFilenames(program.get<std::string>("-output"), path_round_trip_device_xml,
                                        path_round_trip_cluster_xml);

                std::cout << "Saving XML files...." << std::endl;
                //SaveXmlFile(path_round_trip_device_xml.c_str(), device_xml);
                std::cout << "Successfully saved Device XML!" << std::endl;

                //SaveXmlFile(path_round_trip_cluster_xml.c_str(), cluster_xml);
                std::cout << "Successfully saved Cluster XML!" << std::endl;

                if (program.is_used("-validate")){
                    auto path_schema = program.get<std::string>("-validate");
                    std::cout << "Validating output XML files..." << std::endl;
                    if (validateMatter(path_round_trip_device_xml.c_str(),  path_schema.c_str()) == 0)
                        std::cout << "Device XML is valid!" << std::endl;
                    else
                        std::cout << "Device XML is not valid!" << std::endl;
                    if (validateMatter(path_round_trip_cluster_xml.c_str(),  path_schema.c_str()) == 0)
                        std::cout << "Cluster XML is valid!" << std::endl;
                    else
                        std::cout << "Cluster XML is not valid!" << std::endl;
                }

            }else{
                std::string path_sdf_model;
                std::string path_sdf_mapping;
                GenerateSdfFilenames(program.get<std::string>("-output"), path_sdf_model, path_sdf_mapping);

                std::cout << "Saving JSON files...." << std::endl;
                //SaveJsonFile(path_sdf_model.c_str(), sdf_model);
                std::cout << "Successfully saved SDF-Model!" << std::endl;

                //SaveJsonFile(path_sdf_mapping.c_str(), sdf_mapping);
                std::cout << "Successfully saved SDF-Mapping!" << std::endl;

                if (program.is_used("-validate")) {
                    auto path_schema = program.get<std::string>("-validate");
                    std::cout << "Validating output JSON files..." << std::endl;
                    if (validateSdf(path_sdf_model.c_str(), path_schema.c_str()) == 0)
                        std::cout << "SDF-Model JSON is valid!" << std::endl;
                    else
                        std::cout << "SDF-Model JSON is not valid!" << std::endl;
                    if (validateSdf(path_sdf_mapping.c_str(), path_schema.c_str()) == 0)
                        std::cout << "SDF-Mapping JSON is valid!" << std::endl;
                    else
                        std::cout << "SDF-Mapping JSON is not valid!" << std::endl;
                }
            }
        } else {
            std::cerr << "No valid combination of input parameters used" << std::endl;
            std::exit(1);
        }
    }
    else if(program.is_used("--sdf-to-matter")) {
        if (!(program.is_used("-sdf-model") and program.is_used("-sdf-mapping"))) {
            std::cerr << "SDF Model or SDF Mapping missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_sdf_model = program.get<std::string>("-sdf-model");
        auto path_sdf_mapping = program.get<std::string>("-sdf-mapping");
        std::string path_device_xml = "./device.xml";
        std::string path_cluster_xml = "./cluster.xml";
        sdf_to_matter(path_sdf_model, path_sdf_mapping, path_device_xml, path_cluster_xml);
    }
    // Print help of neither convert-to-sdf nor convert-to-matter are given
    else{
        std::cout << program;
    }

    return 0;
}
