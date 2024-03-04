#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
//TODO: Currently using Pugixml for performance und footprint, might make validation optional (Xerces) to safe resources
#include <pugixml.hpp>
#include <argparse/argparse.hpp>
#include <converter.h>

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

    if(program["--convert-to-sdf"] == true)
    {
        if(!(program["-device-xml"] == true and program["-cluster-xml"] == true))
        {
            std::cerr << "Device XML or Cluster XML missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_device_xml = program.get<const char>("-device-xml");
        pugi::xml_document device_xml;
        loadXmlFile(&path_device_xml, device_xml);
        auto path_cluster_xml = program.get<const char>("-cluster-xml");
        pugi::xml_document cluster_xml;
        loadXmlFile(&path_cluster_xml, device_xml);
        convertMatterToSdf(device_xml, cluster_xml);

    }
    if(program["--convert-to-matter"] == true)
    {
        if(!(program["-sdf-model"] == true and program["-sdf-mapping"] == true))
        {
            std::cerr << "SDF Model or SDF Mapping missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path_sdf_model = program.get<const char>("-sdf-model");
        json sdf_model;
        loadJsonFile(&path_sdf_model, sdf_model);
        auto path_sdf_mapping = program.get<const char>("-sdf-mapping");
        json sdf_mapping;
        loadJsonFile(&path_sdf_mapping, sdf_mapping);
        convertSdfToMatter(sdf_model, sdf_mapping);
    }

    if(program["--validate-sdf"] == true)
    {

    }

    return 0;
}
