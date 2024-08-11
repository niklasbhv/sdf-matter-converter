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

//! Function used to load a JSON file from the given path
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

//! Function used to validate a json file against a json schema
int ValidateSdf(const char* path, const char* schema_path)
{
    //Load the json file as well as the schema_path
    nlohmann::ordered_json json_file;
    LoadJsonFile(path, json_file);
    nlohmann::ordered_json json_schema;
    LoadJsonFile(schema_path, json_schema);

    // Create a new validator and set its schema_path
    json_validator validator;
    try {
        validator.set_root_schema(json_schema);
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema_path failed: " << e.what() << "\n";
        return -1;
    }

    // Validate the json file against the schema_path
    try {
        auto default_patch = validator.validate(json_file);
    } catch (const std::exception &e) {
        std::cerr << "Validation of schema_path failed: " << e.what() << "\n";
        return -1;
    }
    return 0;
}

//! Function used to validate a xml file against a xsd schema
int ValidateMatter(const char* path, const char* schema_path) {
    // Try to load the xml file
    xmlDocPtr doc = xmlReadFile(path, NULL, 0);
    if (doc == nullptr) {
        std::cerr << "Failed to parse " << path << std::endl;
        return false;
    }

    // Create a new schema parser context from the xsd schema file
    xmlSchemaParserCtxtPtr parser_ctxt = xmlSchemaNewParserCtxt(schema_path);
    if (parser_ctxt == nullptr) {
        std::cerr << "Could not create XML Schema parser context for " << schema_path << std::endl;
        xmlFreeDoc(doc);
        return false;
    }

    // Create a new schema from the schema parser context
    xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
    if (schema == nullptr) {
        std::cerr << "Failed to parse XML Schema " << schema << std::endl;
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(doc);
        return false;
    }

    // Create a new schema validation context
    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == nullptr) {
        std::cerr << "Could not create XML Schema validation context" << std::endl;
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(parser_ctxt);
        xmlFreeDoc(doc);
        return false;
    }

    // Validate the file against the schema using the validation context
    int ret = xmlSchemaValidateDoc(valid_ctxt, doc);
    if (ret == 0) {
        std::cout << "The XML file is valid against the schema." << std::endl;
    } else if (ret > 0) {
        std::cout << "The XML file is NOT valid against the schema." << std::endl;
    } else {
        std::cerr << "Validation generated an internal error." << std::endl;
    }

    // Cleanup
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(parser_ctxt);
    xmlFreeDoc(doc);

    return ret;
}
