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

//! Max and Min Type boundaries if value is not nullable
//! For nullable values, max has to be decreased by one
#define MATTER_INT_8_MIN 0
#define MATTER_INT_8_MAX 255
#define MATTER_INT_16_MIN 0
#define MATTER_INT_16_MAX 65535
#define MATTER_INT_24_MIN 0
#define MATTER_INT_24_MAX 16777215
#define MATTER_INT_32_MIN 0
#define MATTER_INT_32_MAX 4294967295
#define MATTER_INT_40_MIN 0
#define MATTER_INT_40_MAX 1099511627775
#define MATTER_INT_48_MIN 0
#define MATTER_INT_48_MAX 281474976710655
#define MATTER_INT_56_MIN 0
#define MATTER_INT_56_MAX 72057594037927935
#define MATTER_INT_64_MIN 0
#define MATTER_INT_64_MAX 18446744073709551615

//! Max and Min Type boundaries if value is not nullable
//! For nullable values, min has to be increased by one
#define MATTER_U_INT_8_MIN -128
#define MATTER_U_INT_8_MAX 127
#define MATTER_U_INT_16_MIN -32768
#define MATTER_U_INT_16_MAX 32767
#define MATTER_U_INT_24_MIN -8388608
#define MATTER_U_INT_24_MAX 8388607
#define MATTER_U_INT_32_MIN -2147483648
#define MATTER_U_INT_32_MAX 2147483647
#define MATTER_U_INT_40_MIN -549755813888
#define MATTER_U_INT_40_MAX 549755813887
#define MATTER_U_INT_48_MIN -140737488355328
#define MATTER_U_INT_48_MAX 140737488355327
#define MATTER_U_INT_56_MIN -36028797018963968
#define MATTER_U_INT_56_MAX 36028797018963967
#define MATTER_U_INT_64_MIN -9223372036854775808
#define MATTER_U_INT_64_MAX 9223372036854775807

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

template <matter_data_type> struct MapToMatterDataType_t;
template <> struct MapToMatterDataType_t<matter_data_type::BOOL> { using type = bool; };

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

/*
 *     void setDefault(pugi::xml_attribute default_node) {
        switch (type) {
            case (BOOL):
                default_ = default_node.as_bool();
                break;
            case (MAP8):
                default_ = default_node.as_int();
                break;
            case (MAP16):
                default_ = default_node.as_int();
                break;
            case (MAP32):
                default_ = default_node.as_int();
                break;
            case (MAP64):
                default_ = default_node.as_int();
                break;
            case (UINT8):
                default_ = default_node.as_uint();
                break;
            case (UINT16):
                default_ = default_node.as_uint();
                break;
            case (UINT24):
                default_ = default_node.as_uint();
                break;
            case (UINT32):
                default_ = default_node.as_uint();
                break;
            case (UINT40):
                default_ = default_node.as_uint();
                break;
            case (UINT48):
                default_ = default_node.as_uint();
                break;
            case (UINT56):
                default_ = default_node.as_uint();
                break;
            case (UINT64):
                default_ = default_node.as_uint();
                break;
            case (INT8):
                default_ = default_node.as_int();
                break;
            case (INT16):
                default_ = default_node.as_int();
                break;
            case (INT24):
                default_ = default_node.as_int();
                break;
            case (INT32):
                default_ = default_node.as_int();
                break;
            case (INT40):
                default_ = default_node.as_int();
                break;
            case (INT48):
                default_ = default_node.as_int();
                break;
            case (INT56):
                default_ = default_node.as_int();
                break;
            case (INT64):
                default_ = default_node.as_int();
                break;
            case (SINGLE):
                default_ = default_node.as_float();
                break;
            case (DOUBLE):
                default_ = default_node.as_double();
                break;
            case (OCTSTR):
                default_ = default_node.value();
                break;
            case (LIST):
            case (STRUCT):
            default:
                default_ = default_node.value();
        }

            template<typename T> auto getDefault(){
        if (std::holds_alternative<unsigned int>(default_)) {
            return std::get<unsigned int>(default_);
        } else if (std::holds_alternative<int>(default_)) {
            return std::get<int>(default_);
        } else if (std::holds_alternative<double>(default_)) {
            return std::get<bool>(default_);
        } else if (std::holds_alternative<std::string>(default_)) {
            return std::get<std::string>(default_);
        } else if (std::holds_alternative<bool>(default_)) {
            return std::get<bool>(default_);
        }
            if (std::holds_alternative<unsigned int>(attribute.default_)) {
        std::cout << std::get<unsigned int>(attribute.default_) << std::endl;
    } else if (std::holds_alternative<int>(attribute.default_)) {
        std::cout << std::get<int>(attribute.default_) << std::endl;
    } else if (std::holds_alternative<double>(attribute.default_)) {
        std::cout << std::get<double>(attribute.default_) << std::endl;
    } else if (std::holds_alternative<std::string>(attribute.default_)) {
        std::cout << std::get<std::string>(attribute.default_) << std::endl;
    } else if (std::holds_alternative<bool>(attribute.default_)) {
        std::cout << std::get<bool>(attribute.default_) << std::endl;
    }
 */

#endif //MATTER_CONSTANTS_H
