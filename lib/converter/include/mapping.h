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

/**
 * @file
 * @author Niklas Meyer <nik_mey@uni-bremen.de>
 *
 * @section Description
 *
 * Helper structures used by both conversion directions.
 */

#ifndef SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_MAPPING_H_
#define SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_MAPPING_H_

#include <iostream>
#include <list>
#include "sdf.h"

//! Function to escape JSON Pointer according to section 3 of RFC 6901
//! This function replaces the character `~` with `~0` and `/` with `~1`
//!
//! @param input The string that will be escaped
//! @return The escaped string
inline std::string EscapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find('~', pos)) != std::string::npos) {
        result.replace(pos, 1, "~0");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find('/', pos)) != std::string::npos) {
        result.replace(pos, 1, "~1");
        pos += 2;
    }

    return result;
}

//! Node structure that is used to build the ReferenceTree
class ReferenceTreeNode {
public:
    //! Name of the node
    std::string name;
    //! List of attributes for the node
    std::unordered_map<std::string, sdf::MappingValue> attributes;
    //! Pointer to the parent of the node
    ReferenceTreeNode* parent;
    //! List of pointers to the children of the node
    std::vector<ReferenceTreeNode*> children;

    //! Constructor
    ReferenceTreeNode(std::string name) : name(std::move(name)), attributes(), parent(nullptr) {}

    //! Function used to add a new child to the node
    void AddChild(ReferenceTreeNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    //! Function used to add an attribute to the node
    //! Attributes are a key value pair
    void AddAttribute(const std::string& key, sdf::MappingValue value) {
        attributes[key] = std::move(value);
    }

    //! Function used to generate a pointer that is compliant with section 3 of RFC6901
    std::string GeneratePointer() {
        std::string path;
        ReferenceTreeNode* current = this;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
    }
};

//! Tree structure used for generating the sdf-mapping
class ReferenceTree {
public:
    //! Root node of the tree
    ReferenceTreeNode* root;

    ReferenceTree() {
        root = new ReferenceTreeNode("#");
    }

    //! Function used to generate the complete map section of a sdf-mapping based on the contents of the tree
    std::unordered_map<std::string, std::unordered_map<std::string, sdf::MappingValue>> GenerateMapping(
        ReferenceTreeNode* node) {
        std::unordered_map<std::string, std::unordered_map<std::string, sdf::MappingValue>> map;
        ReferenceTreeNode* current = node;
        for (const auto& child : current->children) {
            if (!child->attributes.empty()) {
                map[GeneratePointer(child)] = child->attributes;
            }
            map.merge(GenerateMapping(child));
        }
        return map;
    }

    //! Function used to generate a pointer that is compliant with section 3 of RFC6901
    std::string GeneratePointer(ReferenceTreeNode* node) {
        std::string path;
        ReferenceTreeNode* current = node;
        while (current != nullptr) {
            path = EscapeJsonPointer(current->name) + (path.empty() ? "" : "/" + path);
            current = current->parent;
        }
        return path;
    }
};

//! Helper function used to determine of a list of strings contains a certain string
//!
//! @param list The list to search through
//! @param str The string to search for
//! @return True if the list contains the string, false otherwise
static bool contains(const std::list<std::string>& list, const std::string& str) {
    return std::find(list.begin(), list.end(), str) != list.end();
}

//! Helper function used to check if a int and a uint are equal
//!
//! @param a The int64_t
//! @param b The uint64_t
//! @return True if the values are equal, false otherwise
inline bool equals(int64_t a, uint64_t b) {
    if (a < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    auto ua = static_cast<uint64_t>(a);
    return ua == b;
}

//! Helper function used to check if a uint64_t and a int64_t are equal
//!
//! @param a The uint64_t
//! @param b The int64_t
//! @return True if the values are equal, false otherwise
inline bool equals(uint64_t a, int64_t b) {
    // Check if the int64_t value is negative
    if (b < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    // At this point, b is non-negative, so we can safely cast to uint64_t
    auto ub = static_cast<uint64_t>(b);
    return a == ub;
}

//! Helper function used to compare a int64_t and a uint64_t
//!
//! @param a The int64_t
//! @param b The uint64_t
//! @return True if the int64_t is smaller or equal to the uint64_t, false otherwise
inline bool compare(int64_t a, uint64_t b) {
    // Check if the int64_t value is negative
    if (a < 0) {
        // A negative int64_t is always less than any uint64_t
        return true;
    }
    // At this point, a is non-negative, so we can safely cast to uint64_t
    auto ua = static_cast<uint64_t>(a);
    return ua <= b;
}

//! Helper function used to compare a uint64_t and a int64_t
//!
//! @param a The uint64_t
//! @param b The int64_t
//! @return True if the uint64_t is smaller or equal to the int64_t, false otherwise
inline bool compare(uint64_t a, int64_t b) {
    // Check if the int64_t value is negative
    if (b < 0) {
        // A negative int64_t is always less than any uint64_t
        return false;
    }
    // At this point, b is non-negative, so we can safely cast to uint64_t
    auto ub = static_cast<uint64_t>(b);
    return a <= ub;
}

//! Helper function used to get the remaining string after a slash
//!
//! @param str Input string
//! @return Remaining string after the slash or the original string, if no slash was found
inline std::string GetLastPartAfterSlash(const std::string& str) {
    size_t pos = str.find_last_of('/');
    if (pos != std::string::npos) {
        return str.substr(pos + 1);
    }
    return str;  // If no slash is found, return the original string
}

#endif //SDF_MATTER_CONVERTER_LIB_CONVERTER_INCLUDE_MAPPING_H_
