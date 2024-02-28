#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
//TODO: Currently using Pugixml for performance und footprint, might make validation optional (Xerces) to safe resources
#include <pugixml.hpp>
#include <argparse/argparse.hpp>

using json = nlohmann::json;

int ReadJsonFile(const std::string& path)
{
    //TODO: Path given is only to the folder
    std::ifstream f(path);
    //TODO: Check if this can fail (and how)
    json data = json::parse(f);
    return 0;
}

int ReadXmlFile(const char *path)
{
    pugi::xml_document doc;
    //TODO: Path given is only to the folder
    pugi::xml_parse_result result = doc.load_file(path);
    if (!result)
        return -1;
    return 0;
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("sdf-matter-converter");

    program.add_argument("--convert-to-sdf")
        .help("Convert from Matter to SDF");

    program.add_argument("--convert-to-matter")
        .help("Convert from SDF to Matter");

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

        auto path = program.get<const char>("-path");
        int result = ReadXmlFile(&path);
    }
    if(program["--convert-to-matter"] == true)
    {
        if(!(program["-sdf-model"] == true and program["-sdf-mapping"] == true))
        {
            std::cerr << "SDF Model or SDF Mapping missing as an input argument" << std::endl;
            std::exit(1);
        }

        auto path = program.get<std::string>("-path");
        int result = ReadJsonFile(path);
    }

    return 0;
}
