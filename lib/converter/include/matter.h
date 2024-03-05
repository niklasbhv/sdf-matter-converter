//
// Created by niklas on 05.03.24.
//

#ifndef MATTER_H
#define MATTER_H

#include <string>
#include <map>
#include <list>

struct bitmapType{
    std::string name;
    std::string type;
    std::string cluster;
    std::map<std::string, std::string> fields;
};

struct enumType{
    std::string name;
    std::string type;
    std::string cluster;
    std::map<std::string, std::string> items;
};

struct eventFieldType{
    std::string id;
    std::string name;
    std::string type;
    bool array;
    int length;
};

struct eventType {
    std::string code;
    std::string name;
    std::string priority;
    std::string side;
    bool optional;
    std::string description;
    std::list<eventFieldType> fields;
};

struct commandType {
    std::string source;
    std::string code;
    std::string name;
    bool optional;
    std::string description;

};

struct attributeType {
    std::string side;
    std::string description;
    std::map<std::string, std::string> access;
    std::string code;
    std::string define;
    std::string type;
    std::string deflt; //TODO: Temporary
    std::string entityType;
    bool reportable;
    bool writable;
    bool optional;
    bool isNullable;
    int min;
    int max;
    int length;
};

struct clusterType {
    std::string name;
    //TODO: Can this be solved like this?
    std::string server;
    std::string client;
    std::string domain;
    std::string code;
    std::string define;
    std::string description;
    attributeType attributes[10];
    commandType commands[10];
    eventType events[10];
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
