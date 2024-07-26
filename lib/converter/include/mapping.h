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

#ifndef MAPPING_H
#define MAPPING_H

#include <list>
#include "sdf.h"

//! Function to unescape JSON Pointer according to RFC 6901
inline std::string UnescapeJsonPointer(const std::string& input) {
    std::string result = input;
    std::size_t pos = 0;
    while ((pos = result.find("~1", pos)) != std::string::npos) {
        result.replace(pos, 2, "/");
        pos += 1;
    }
    pos = 0;
    while ((pos = result.find("~0", pos)) != std::string::npos) {
        result.replace(pos, 2, "~");
        pos += 1;
    }

    return result;
}

//! Function to escape JSON Pointer according to RFC 6901
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

class ReferenceTreeNode {
public:
    std::string name;
    std::unordered_map<std::string, sdf::MappingValue> attributes;
    ReferenceTreeNode* parent;
    std::vector<ReferenceTreeNode*> children;

    ReferenceTreeNode(std::string name) : name(std::move(name)), attributes(), parent(nullptr) {}

    void AddChild(ReferenceTreeNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    void AddAttribute(const std::string& key, sdf::MappingValue value) {
        attributes[key] = std::move(value);
    }

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

class ReferenceTree {
public:
    ReferenceTreeNode* root;

    ReferenceTree() {
        root = new ReferenceTreeNode("#");
    }

    std::unordered_map<std::string, std::unordered_map<std::string, sdf::MappingValue>> GenerateMapping(ReferenceTreeNode* node) {
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

static bool contains(const std::list<std::string>& list, const std::string& str) {
    return std::find(list.begin(), list.end(), str) != list.end();
}

#endif //MAPPING_H
