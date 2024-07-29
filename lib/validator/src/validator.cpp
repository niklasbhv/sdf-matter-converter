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

#include <nlohmann/json-schema.hpp>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <fstream>
#include <iostream>

using nlohmann::ordered_json;
using nlohmann::json_schema::json_validator;

int LoadJsonFile(const char* path, nlohmann::ordered_json& json_file)
{
    try {
        std::ifstream f(path);
        json_file = nlohmann::ordered_json::parse(f);
    }
    catch (const std::exception& err) {
        std::cerr << "Failed to load JSON file: " << path << std::endl;
        std::cerr << err.what() << std::endl;
        return -1;
    }
    return 0;
}

int ValidateSdf(const char* path, const char* schema)
{
    //Load the json file as well as the schema
    nlohmann::ordered_json json_file;
    LoadJsonFile(path, json_file);
    nlohmann::ordered_json json_schema;
    LoadJsonFile(schema, json_schema);

    // Create a new validator and set its schema
    json_validator validator;
    try {
        validator.set_root_schema(json_schema);
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema failed: " << e.what() << "\n";
        return -1;
    }

    // Validate the json file against the schema
    try {
        auto defaultPatch = validator.validate(json_file);
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema failed: " << e.what() << "\n";
        return -1;
    }
    return 0;
}

int ValidateMatter(const char* xmlFile, const char* schemaFile) {
    xmlDocPtr doc = xmlReadFile(xmlFile, NULL, 0);
    if (doc == nullptr) {
        std::cerr << "Failed to parse " << xmlFile << std::endl;
        return false;
    }

    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewParserCtxt(schemaFile);
    if (parserCtxt == nullptr) {
        std::cerr << "Could not create XML Schema parser context for " << schemaFile << std::endl;
        xmlFreeDoc(doc);
        return false;
    }

    xmlSchemaPtr schema = xmlSchemaParse(parserCtxt);
    if (schema == nullptr) {
        std::cerr << "Failed to parse XML Schema " << schemaFile << std::endl;
        xmlSchemaFreeParserCtxt(parserCtxt);
        xmlFreeDoc(doc);
        return false;
    }

    xmlSchemaValidCtxtPtr validCtxt = xmlSchemaNewValidCtxt(schema);
    if (validCtxt == nullptr) {
        std::cerr << "Could not create XML Schema validation context" << std::endl;
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(parserCtxt);
        xmlFreeDoc(doc);
        return false;
    }

    int ret = xmlSchemaValidateDoc(validCtxt, doc);
    if (ret == 0) {
        std::cout << "The XML file is valid against the schema." << std::endl;
    } else if (ret > 0) {
        std::cout << "The XML file is NOT valid against the schema." << std::endl;
    } else {
        std::cerr << "Validation generated an internal error." << std::endl;
    }

    xmlSchemaFreeValidCtxt(validCtxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(parserCtxt);
    xmlFreeDoc(doc);

    return ret;
}