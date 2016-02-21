//
//  structures.h
//  AKVS
//
//  Created by Aitocir on 10/28/15.
//  Copyright © 2015 AitocirLabs. All rights reserved.
//

#ifndef structures_h
#define structures_h

#include <string>

#include "config.h"

namespace AKVSStructures {
    
    // useful for results from key search
    // if .exists, index is location (tail is useless, but will contain pointer to it)
    // else, index is first available index in hash bucket (EOF if there is none)
    //  and tail is the currently NULL pointer that will need to point to the newly added key
    struct key_idx {
        bool exists;
        uint32_t index;
        uint32_t tail;
    };
    
    struct key_value {
        char* key = NULL;
        AKVSConfig::value_type type = AKVSConfig::value_type::EMPTY;
        char* value = NULL;
        
        void setNonexistantKVPair() {
            type = AKVSConfig::value_type::EMPTY;
        }
        
        void setKey(std::string s){
            key = new char[AKVSConfig::CONFIG_keySize+1];
            for(int i=0; i<AKVSConfig::CONFIG_keySize+1; i++) {
                if (i<s.size()){
                    key[i] = s[i];
                } else {
                    key[i] = '\0';
                }
            }
        }
        
        void preparePair(int keySize, int valueSize) {
            if(key!=NULL) {
                delete [] key;
            }
            if(value!=NULL){
                delete [] value;
            }
            
            key = new char[keySize+1];
            value = new char[valueSize+1];
        }
        
        uint8_t valueAs_uint8() {
            if (type != AKVSConfig::value_type::UINT8) {
                return 0;
            }
            else {
                uint8_t val = 0;
                val |= (uint8_t)value[0];
                return val;
            }
        }
        
        uint16_t valueAs_uint16() {
            if (type != AKVSConfig::value_type::UINT16) {
                return 0;
            }
            else {
                uint16_t val = 0;
                val |= (uint16_t)((uint8_t)value[0]);
                val <<= 8;
                val |= (uint16_t)((uint8_t)value[1]);
                return val;
            }
        }
        
        uint32_t valueAs_uint32() {
            if (type != AKVSConfig::value_type::UINT32) {
                return 0;
            }
            else {
                uint32_t val = 0;
                for(int i=0; i<4; i++) {
                    val <<= 8;
                    val |= (uint32_t)((uint8_t)value[i]);
                }
                return val;
            }
        }
        
        uint64_t valueAs_uint64() {
            if (type != AKVSConfig::value_type::UINT64) {
                return 0;
            }
            else {
                uint64_t val = 0;
                for(int i=0; i<8; i++) {
                    val <<= 8;
                    val |= (uint64_t)((uint8_t)value[i]);
                }
                return val;
            }
        }
        
        int8_t valueAs_int8() {
            if (type != AKVSConfig::value_type::INT8) {
                return INT8_MIN;
            }
            else {
                int8_t val = 0;
                val |= value[0];
                return val;
            }
        }
        
        int16_t valueAs_int16() {
            if (type != AKVSConfig::value_type::INT16) {
                return INT16_MIN;
            }
            else {
                int16_t val = 0;
                val |= (uint16_t)((uint8_t)value[0]);
                val <<= 8;
                val |= (uint16_t)((uint8_t)value[1]);
                return val;
            }
        }
        
        int32_t valueAs_int32() {
            if (type != AKVSConfig::value_type::INT32) {
                return INT32_MIN;
            }
            else {
                int32_t val = 0;
                for(int i=0; i<4; i++) {
                    val <<= 8;
                    val |= (uint32_t)((uint8_t)value[i]);
                }
                return val;
            }
        }
        
        int64_t valueAs_int64() {
            if (type != AKVSConfig::value_type::INT64) {
                return INT64_MIN;
            }
            else {
                int64_t val = 0;
                for(int i=0; i<8; i++) {
                    val <<= 8;
                    val |= (uint64_t)((uint8_t)value[i]);
                }
                return val;
            }
        }
        
        // other stuff
        
        bool valueAs_bool() {
            if (type != AKVSConfig::value_type::BOOLEAN) {
                return false;
            }
            else {
                return value[0] != '\0';
            }
        }
        
        std::string valueAs_char16() {
            if (type != AKVSConfig::value_type::CHAR16) {
                return "";
            }
            else {
                return std::string(value);
            }
        }
        
        std::string valueAs_char64() {
            if (type != AKVSConfig::value_type::CHAR64) {
                return "";
            }
            else {
                return std::string(value) ;
            }
        }
        
        std::string valueAs_char256() {
            if (type != AKVSConfig::value_type::CHAR256) {
                return "";
            }
            else {
                return std::string(value) ;
            }
        }
        
        std::string valueAs_char1024() {
            if (type != AKVSConfig::value_type::CHAR1024) {
                return "";
            }
            else {
                return std::string(value) ;
            }
        }
        
        float valueAs_float() {
            if (type != AKVSConfig::value_type::FLOAT) {
                return 0.0;
            }
            else {
                char* end;
                float f = strtof(value, &end);
                if(end == value) {
                    return 0.0;
                }
                return f;
            }
        }
        
        double valueAs_double() {
            if (type != AKVSConfig::value_type::DOUBLE) {
                return 0.0;
            }
            else {
                char* end;
                double d = strtod(value, &end);
                if(end == value) {
                    return 0.0;
                }
                return d;
            }
        }
        
        std::string valueAsString() {
            
            switch (type) {
                case AKVSConfig::value_type::UINT8:
                    return std::to_string(valueAs_uint8());
                case AKVSConfig::value_type::UINT16:
                    return std::to_string(valueAs_uint16());
                case AKVSConfig::value_type::UINT32:
                    return std::to_string(valueAs_uint32());
                case AKVSConfig::value_type::UINT64:
                    return std::to_string(valueAs_uint64());
                    
                case AKVSConfig::value_type::INT8:
                    return std::to_string(valueAs_int8());
                case AKVSConfig::value_type::INT16:
                    return std::to_string(valueAs_int16());
                case AKVSConfig::value_type::INT32:
                    return std::to_string(valueAs_int32());
                case AKVSConfig::value_type::INT64:
                    return std::to_string(valueAs_int64());
                    
                case AKVSConfig::value_type::FLOAT:
                    return std::to_string(valueAs_float());
                case AKVSConfig::value_type::DOUBLE:
                    return std::to_string(valueAs_double());
                    
                case AKVSConfig::value_type::BOOLEAN:
                    return std::to_string(valueAs_bool());
                    
                case AKVSConfig::value_type::CHAR16:
                    return valueAs_char16();
                case AKVSConfig::value_type::CHAR64:
                    return valueAs_char64();
                case AKVSConfig::value_type::CHAR256:
                    return valueAs_char256();
                case AKVSConfig::value_type::CHAR1024:
                    return valueAs_char1024();
                    
                default:
                    return "(null)";
            }
        }
    };
    
    struct cmd_result {
        int code;
        std::string message;
        key_value content;
        
        std::string contentString() {
            std::string self = "key: ";
            self += std::string(content.key);
            self += "\ntype: ";
            self += getNameForValueType(content.type);
            self += "\nvalue: ";
            self += content.valueAsString();
            self += "\n";
            return self;
        }
    };
}

#endif /* structures_h */
