//
// Created by Niklas on 26.02.2024.
//
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

int loadFile(const char* path)
{
    //TODO: Path given is only to the folder
    std::ifstream f(path);
    //TODO: Check if this can fail (and how)
    json data = json::parse(f);
    return 0;
}

int validateSdf(const char* path, const char* schema)
{

    return 0;
}

int validateMatter()
{
    return 0;
}