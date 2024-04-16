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
 * Constants used for Matter.
 */
#ifndef MATTER_CONSTANTS_H
#define MATTER_CONSTANTS_H

enum matter_data_type {
    BOOL,
    MAP8,
    MAP16,
    MAP32,
    MAP64,
    UINT8,
    UINT16,
    UINT24,
    UINT32,
    UINT40,
    UINT48,
    UINT56,
    UINT64,
    INT8,
    INT16,
    INT24,
    INT32,
    INT40,
    INT48,
    INT56,
    INT64,
    SINGLE,
    DOUBLE,
    OCTSTR,
    LIST,
    STRUCT,
    UNKNOWN,
};

inline std::string toString(matter_data_type data_type) {
    switch (data_type)
    {
        case (BOOL):
            return "bool";
        case (MAP8):
            return "map8";
        case (MAP16):
            return "map16";
        case (MAP32):
            return "map32";
        case (MAP64):
            return "map64";
        case (UINT8):
            return "uint8";
        case (UINT16):
            return "uint16";
        case (UINT24):
            return "uint24";
        case (UINT32):
            return "uint32";
        case (UINT40):
            return "uint40";
        case (UINT48):
            return "uint48";
        case (UINT56):
            return "uint56";
        case (UINT64):
            return "uint64";
        case (INT8):
            return "int8";
        case (INT16):
            return "int16";
        case (INT24):
            return "int24";
        case (INT32):
            return "int32";
        case (INT40):
            return "int40";
        case (INT48):
            return "int48";
        case (INT56):
            return "int56";
        case (INT64):
            return "int64";
        case (SINGLE):
            return "single";
        case (DOUBLE):
            return "double";
        case (OCTSTR):
            return "octstr";
        case (LIST):
            return "list";
        case (STRUCT):
            return "struct";
        default:
            return "unknown";
    }
}

inline matter_data_type fromString(const std::string& data_type) {
    if (data_type == "bool")
        return BOOL;
    if (data_type == "map8")
        return MAP8;
    if (data_type == "map16")
        return MAP16;
    if (data_type == "map32")
        return MAP32;
    if (data_type == "map64")
        return MAP64;
    if (data_type == "uint8")
        return UINT8;
    if (data_type == "uint16")
        return UINT16;
    if (data_type == "uint24")
        return UINT24;
    if (data_type == "uint32")
        return UINT32;
    if (data_type == "uint40")
        return UINT40;
    if (data_type == "uint48")
        return UINT48;
    if (data_type == "uint56")
        return UINT56;
    if (data_type == "uint64")
        return UINT64;
    if (data_type == "int8")
        return INT8;
    if (data_type == "int16")
        return INT16;
    if (data_type == "int24")
        return INT24;
    if (data_type == "int32")
        return INT32;
    if (data_type == "int40")
        return INT40;
    if (data_type == "int48")
        return INT48;
    if (data_type == "int56")
        return INT56;
    if (data_type == "int64")
        return INT64;
    if (data_type == "single")
        return SINGLE;
    if (data_type == "double")
        return DOUBLE;
    if (data_type == "octstr")
        return OCTSTR;
    if (data_type == "list")
        return LIST;
    if (data_type == "struct")
        return STRUCT;
    return UNKNOWN;
}

enum matter_priority_type {
    debug = 0,
    info = 1,
    critical = 2,
};

enum numeric_data_type {
};

enum octet_string_data_type {};

enum list_data_type {};

#endif //MATTER_CONSTANTS_H
