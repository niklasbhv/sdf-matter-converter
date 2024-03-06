//
// Created by niklas on 05.03.24.
//

#ifndef MATTER_H
#define MATTER_H

#include <string>
#include <map>
#include <list>

struct accessType{
    std::string op;
    std::string role;
    std::string privilege;
    std::string modifier;
};

struct fieldType{
    std::string mask; // required
    std::string name; //required
    std::string introducedIn; //type: zxlSpecVersion
    int fieldId;
};

struct bitmapType{
    std::string name; // required
    std::string type; // required
    std::string cluster; // min = 0; max = inf; name = cluster
    std::list<fieldType> fields; // min = 1; max = inf; ref=field
};

struct enumType{
    std::string name;
    std::string type;
    std::string cluster;
    std::map<std::string, std::string> items;
};

struct eventFieldType{
    std::string id; // required
    std::string name; // required
    std::string type; // required
    bool array; //optional
    bool isNullable; //optional
    //int length; TODO: Does not seem to be part of the schema
};

struct eventType {
    // ref=description
    std::list<accessType> access; // min = 0; max = inf
    std::list<eventFieldType> field; // min = 0; max = inf
    std::string code;
    std::string name;
    std::string side; // required; either server or client
    std::string priority;
    //bool optional; TODO: Does not seem to be part of the schema
};

struct commandType {
    // ref=description
    std::list<accessType> access; // min = 0; max = inf
    //std::list<argType> arg; // min = 0; max = inf
    //cli
    std::string cliFunctionName;
    std::string code;
    bool disableDefaultResponse;
    std::string functionName;
    std::string group;
    std::string introducedIn; //type: zxlSpecVersion
    bool noDefaultImplementation;
    std::string manufacturerCode;
    std::string name;
    bool optional;
    std::string source;
    std::string restriction;
    std::string response;
};

struct attributeType {
    // ref=description can be multiple or none
    std::list<accessType> access; // min = 0; max = inf
    std::string code; // zclCode
    // default
    std::string define; // required; zclAttributeDefine
    std::string introducedIn; // zclSpecVersion
    int length;
    // manufacturerCode // zclCode
    int max; //TODO: Check anySimpleType
    int min; //TODO: Check anySimpleType
    // reportMaxInterval //TODO: Check anySimpleType
    // reportMinInterval
    // reportableChange //TODO: Check anySimpleType
    bool optional; // required
    std::string side; // required
    std::string type; // required
    bool readable;
    bool writable;
    bool reportable;
    bool array;
    bool isNullable;
};

struct clusterType {
    std::string name; //TODO: Check ref
    std::string domain; //TODO: Check ref
    std::string description; //TODO: Check ref
    std::string code; //TODO: Check ref
    std::string define; //TODO: Check ref
    std::string server; //TODO: Check ref
    std::string client; //TODO: Check ref
    // generateCmdHandlers; min = 0; max = inf
    // tag; type = tag; min = 0; max = inf
    //globalAttribute; type = globalAttribute
    std::list<attributeType> attributes; //min = 0; max = inf
    std::list<commandType> commands; //min = 0; max = inf
    std::list<eventType> events; //min = 0; max = inf
};

struct deviceType {
    std::string name;
    std::string domain;
    std::string typeName;
    std::string profileId;
    std::string deviceId;
    // TODO: Channels is currently missing
    std::list<clusterType> clusters;
};

#endif //MATTER_H
