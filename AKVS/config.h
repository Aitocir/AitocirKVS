//
//  config.h
//  AKVS
//
//  Created by Aitocir on 10/24/15.
//  Copyright © 2015 AitocirLabs. All rights reserved.
//

#ifndef config_h
#define config_h

#include <fstream>
#include <map>
#include <set>

namespace AKVSConfig {
    
    // file
    //uint8_t* dbPtr;
    //int dbSize;
    //int dbFile;
    std::fstream db_file;
    uint32_t db_fileSize;
    
    // global constants
    const uint32_t CONFIG_HASHTABLE_START = 10; // first byte index of hash table right after file header
    
    // global config variables
    uint16_t CONFIG_keySize = 32;
    std::map<int, std::set<int> > CONFIG_weights = {
        {1, {0}},
        {2, {1}},
        {4, {2}},
        {8, {3}},
        {32, {4}},
        {256, {5}},
        {4096, {6}},
        {65536, {7}}
    };
    
    
    // storage type definitions
    enum class value_type : uint8_t {
        
        // unsigned integers
        UINT8 = 0x00,
        UINT16 = 0x01,
        UINT32 = 0x02,
        UINT64 = 0x03,
        
        // signed integers
        INT8 = 0x10,
        INT16 = 0x11,
        INT32 = 0x12,
        INT64 = 0x13,
        
        // floating point numbers
        FLOAT = 0x20,
        DOUBLE = 0x21,
        
        // other primitives
        BOOLEAN = 0x30,
        
        // strings
        CHAR32 = 0x40,
        CHAR256 = 0x41,
        CHAR4096 = 0x42,
        CHAR65536 = 0x43,
        
        // not a valid type symbol
        EMPTY = 0xFF
        
    };
    
    value_type getValueTypeForRawValue(uint8_t raw){
        
        switch (raw) {
            case 0x00:
                return value_type::UINT8;
            case 0x01:
                return value_type::UINT16;
            case 0x02:
                return value_type::UINT32;
            case 0x03:
                return value_type::UINT64;
                
            case 0x10:
                return value_type::INT8;
            case 0x11:
                return value_type::INT16;
            case 0x12:
                return value_type::INT32;
            case 0x13:
                return value_type::INT64;
                
            case 0x20:
                return value_type::FLOAT;
            case 0x21:
                return value_type::DOUBLE;
                
            case 0x30:
                return value_type::BOOLEAN;
                
            case 0x40:
                return value_type::CHAR32;
            case 0x41:
                return value_type::CHAR256;
            case 0x42:
                return value_type::CHAR4096;
            case 0x43:
                return value_type::CHAR65536;
                
            default:
                return value_type::EMPTY;
        }
    }
    
    uint32_t getValueLengthForType(value_type type){
        
        switch (type) {
                
            // 1 byte
            case value_type::UINT8:
            case value_type::INT8:
            case value_type::BOOLEAN:
                return 1;
            
            // 2 bytes
            case value_type::UINT16:
            case value_type::INT16:
                return 2;
                
            // 4 bytes
            case value_type::UINT32:
            case value_type::INT32:
                return 4;
                
            // 8 bytes
            case value_type::UINT64:
            case value_type::INT64:
            case value_type::FLOAT:
                return 8;
                
            // 32 bytes
            case value_type::CHAR32:
            case value_type::DOUBLE:
                return 32;
                
            // 256 bytes
            case value_type::CHAR256:
                return 256;
                
            // 4k bytes
            case value_type::CHAR4096:
                return 4096;
                
            // 4k bytes
            case value_type::CHAR65536:
                return 65536;
                
            case value_type::EMPTY:
                return 0;
        }
    }
    
    value_type getValueTypeByName(std::string name){
        
        std::string nameLower = name;
        
        if (nameLower == "uint8") {
            return value_type::UINT8;
        } else if (nameLower == "uint16") {
            return value_type::UINT16;
        } else if (nameLower == "uint32") {
            return value_type::UINT32;
        } else if (nameLower == "uint64") {
            return value_type::UINT64;
        } else if (nameLower == "int8") {
            return value_type::INT8;
        } else if (nameLower == "int16") {
            return value_type::INT16;
        } else if (nameLower == "int32") {
            return value_type::INT32;
        } else if (nameLower == "int64") {
            return value_type::INT64;
        } else if (nameLower == "float") {
            return value_type::FLOAT;
        } else if (nameLower == "double") {
            return value_type::DOUBLE;
        } else if (nameLower == "bool" || name == "boolean") {
            return value_type::BOOLEAN;
        } else if (nameLower == "char32") {
            return value_type::CHAR32;
        } else if (nameLower == "char256") {
            return value_type::CHAR256;
        } else if (nameLower == "char4096") {
            return value_type::CHAR4096;
        } else if (nameLower == "char65536") {
            return value_type::CHAR65536;
        } else {
            return value_type::EMPTY;
        }
    }
    
    std::string getNameForValueType(value_type type){
        
        switch (type) {
            case value_type::UINT8:
                return "uint8";
            case value_type::UINT16:
                return "uint16";
            case value_type::UINT32:
                return "uint32";
            case value_type::UINT64:
                return "uint64";
                
            case value_type::INT8:
                return "int8";
            case value_type::INT16:
                return "int16";
            case value_type::INT32:
                return "int32";
            case value_type::INT64:
                return "int64";
                
            case value_type::FLOAT:
                return "float";
            case value_type::DOUBLE:
                return "double";
                
            case value_type::BOOLEAN:
                return "boolean";
                
            case value_type::CHAR32:
                return "char32";
            case value_type::CHAR256:
                return "char256";
            case value_type::CHAR4096:
                return "char4096";
            case value_type::CHAR65536:
                return "char65536";
                
            default:
                return "null";
        }
    }
    
}

#endif /* config_h */
