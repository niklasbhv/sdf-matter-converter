#ifndef MATTER_H
#define MATTER_H

#include <string>
#include <map>
#include <list>
#include "pugixml.hpp"

struct argType;

struct groupType{
    std::list<argType> description;
    std::string id;
    std::string name;
};

struct commandType;

struct cliType{
    std::list<groupType> group;
    std::list<commandType> command;
};

struct typeType{
    std::string id; // required
    std::string name; // required
    std::string description; // required
    int size;
    bool discrete;
    bool signed_;
    bool string;
    bool chr;
    bool lng;
    bool analog;
    bool composite;
};

struct olderType{
    std::string dependsOn; // optional
    std::string spec; // optional
    bool certifiable;
};

struct channelType{
    std::list<int> channels; // min = 11; max = 26; whitespace => collapse
    bool editable; // required
};

struct domainType{
    std::list<olderType> older;
    std::string dependsOn; // zclSpecVersion -> Mapping file
    std::string name;
    std::string spec; // zclSpecVersion -> Mapping file
    bool certifiable;
};

struct argType{
    bool arrayLength;
    bool array;
    std::string default_;
    std::string description;
    std::string introducedIn; // zclSpecVersion -> Mapping file
    std::string removedIn; // zclSpecVersion -> Mapping file
    std::string name;
    typeType type;
    int length;
    std::string presentIf;
    int optional;
    int fieldId;
    std::string countArg; // optional
    bool isNullable;
};

struct tagType{
    std::string name; // required
    std::string description; // required
};

struct featureBitType{
    std::string tag; //required
    int bit; //required
};

struct globalAttributeType{
    std::list<featureBitType> featureBit;
    std::string side; // required; zclSideWithEither -> Mapping file
    std::string code; // required
    std::string value; // required
};

struct clientType{
    bool client;
    bool init; // required
    bool tick; // required
};

struct serverType{
    bool server;
    bool init; // required
    bool tick; // required
    std::string tickFrequency;
};

struct accessType{
    std::string op;
    std::string role;
    std::string privilege;
    std::string modifier;
};

struct fieldType{
    std::string mask; // required
    std::string name; // required
    std::string introducedIn; //zclSpecVersion -> Mapping file
    int fieldId;
};

struct bitmapType{
    std::string name; // required
    std::string type; // required
    //TODO: This gets matched to the cluster code attribute
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
};

struct eventType {
    //! Originally std::list<argType>, for now simplified to just the description
    //TODO: You can technically define args inside the description, check if and if yes how this should be handled
    std::string description;
    std::list<accessType> access; // min = 0; max = inf
    std::list<eventFieldType> field; // min = 0; max = inf
    std::string code;
    std::string name;
    std::string side; // required; zclSide
    std::string priority;
};

struct commandType {
    //! Originally std::list<argType>, for now simplified to just the description
    //TODO: You can technically define args inside the description, check if and if yes how this should be handled
    std::string description;
    std::list<accessType> access; // min = 0; max = inf
    std::list<argType> arg; // min = 0; max = inf
    cliType cli;
    std::string cliFunctionName;
    std::string code;
    bool disableDefaultResponse;
    std::string functionName;
    std::string group;
    std::string introducedIn; // zclSpecVersion -> Mapping file
    bool noDefaultImplementation;
    std::string manufacturerCode; // zclCode -> Mapping file
    std::string name;
    bool optional;
    std::string source;
    std::string restriction;
    std::string response;
};

struct attributeType {
    std::string name; //The value of the Attribute XML Element
    //! Originally std::list<argType>, for now simplified to just the description
    //TODO: You can technically define args inside the description, check if and if yes how this should be handled
    std::string description; // min = 0; max = inf
    std::list<accessType> access; // min = 0; max = inf
    std::string code; // zclCode -> Mapping file
    std::string default_;
    std::string define; // required; zclAttributeDefine -> Mapping file
    std::string introducedIn; // zclSpecVersion -> Mapping file
    int length;
    std::string  manufacturerCode; // zclCode -> Mapping file
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
    std::string name;
    //! Originally domainType, for now simplified to just the name
    std::string domain;
    //! Originally std::list<argType>, for now simplified to just the description
    //TODO: You can technically define args inside the description, check if and if yes how this should be handled
    std::string description;
    std::string code; //zclCode -> Mapping file
    std::string define;
    serverType server;
    clientType client;
    bool generateCmdHandlers; // min = 0;
    std::list<tagType> tag;
    std::list<globalAttributeType> globalAttribute;
    std::list<attributeType> attributes; //min = 0; max = inf
    std::list<commandType> commands; //min = 0; max = inf
    std::list<eventType> events; //min = 0; max = inf
    std::string introducedIn; // -> Mapping file
    std::string manufacturerCode; // zclCode -> Mapping file
    bool singleton;
};

struct deviceType {
    std::string name;
    //! Originally domainType, for now simplified to just the name
    std::string domain;
    std::string typeName;
    //! Originally consists of an additional editable field, for now simplified to just the name
    std::string profileId;
    //! Originally consists of an additional editable field, for now simplified to just the name
    std::string deviceId;
    //! Originally channelType, for now simplified to just the channels
    std::list<int> channels; // Each element is between 11 and 26
    std::list<clusterType> clusters;
};

int parseClusters(const pugi::xml_node& cluster_xml, std::list<clusterType>& clusterList);
int parseDevice(const pugi::xml_node& device_xml, deviceType& device);

#endif //MATTER_H
