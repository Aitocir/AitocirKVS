//
//  input_shell.h
//  AKVS
//
//  Created by Aitocir on 10/28/15.
//  Copyright © 2015 AitocirLabs. All rights reserved.
//

#ifndef input_shell_h
#define input_shell_h

#include <iomanip>
#include <sstream>
#include <string>

#include "config.h"
#include "structures.h"


namespace AKVSInputShell {
    
    AKVSStructures::key_value packagePairForUINT(AKVSStructures::key_value keyValue, std::string rawValue, int bytes) {
        
        uint64_t maxAllowed;
        uint64_t mask;
        int offset = (bytes-1) * 8;
        
        if (bytes == 1) {
            maxAllowed = UINT8_MAX;
            mask = 0x00000000000000FF;
            keyValue.type = AKVSConfig::value_type::UINT8;
        }
        else if (bytes == 2) {
            maxAllowed = UINT16_MAX;
            mask = 0x000000000000FF00;
            keyValue.type = AKVSConfig::value_type::UINT16;
        }
        else if (bytes == 4) {
            maxAllowed = UINT32_MAX;
            mask = 0x00000000FF000000;
            keyValue.type = AKVSConfig::value_type::UINT32;
        }
        else if (bytes == 8) {
            maxAllowed = UINT64_MAX;
            mask = 0xFF00000000000000;
            keyValue.type = AKVSConfig::value_type::UINT64;
        } else {
            keyValue.type = AKVSConfig::value_type::EMPTY;
            return keyValue;
        }
        
        uint64_t value = 0;
        for (int i=0; i<rawValue.size(); i++) {
            if (rawValue[i] >= 0x30 && rawValue[i] <= 0x39) {
                value *= 10;
                value += (rawValue[i] - 0x30);
            }
            else {
                // invalid character provided
                keyValue.type = AKVSConfig::value_type::EMPTY;
                return keyValue;
            }
            
            if (value > maxAllowed) {
                // oops, value is too large!
                keyValue.type = AKVSConfig::value_type::EMPTY;
                return keyValue;
            }
        }
        
        keyValue.value = new char[bytes];
        for(int i=0; i<bytes; i++) {
            keyValue.value[i] = '\0';
            keyValue.value[i] |= (uint8_t)((value & mask) >> offset);
            mask >>= 8;
            offset -= 8;
        }
        
        return keyValue;
    }
    
    AKVSStructures::key_value packagePairForINT(AKVSStructures::key_value keyValue, std::string rawValue, int bytes) {
        
        int64_t maxAllowed;
        int64_t mask;
        int offset = (bytes-1) * 8;
        
        if (bytes == 1) {
            maxAllowed = INT8_MAX;
            mask = 0x00000000000000FF;
            keyValue.type = AKVSConfig::value_type::INT8;
        }
        else if (bytes == 2) {
            maxAllowed = INT16_MAX;
            mask = 0x000000000000FF00;
            keyValue.type = AKVSConfig::value_type::INT16;
        }
        else if (bytes == 4) {
            maxAllowed = INT32_MAX;
            mask = 0x00000000FF000000;
            keyValue.type = AKVSConfig::value_type::INT32;
        }
        else if (bytes == 8) {
            maxAllowed = INT64_MAX;
            mask = 0xFF00000000000000;
            keyValue.type = AKVSConfig::value_type::INT64;
        } else {
            keyValue.type = AKVSConfig::value_type::EMPTY;
            return keyValue;
        }
        
        int64_t value = 0;
        int i=0;
        bool isNegative = false;
        if (rawValue.size() > 0 && rawValue[0] == '-'){
            i=1;
            isNegative = true;
        }
        else if (rawValue.size() > 0 && rawValue[0] == '+'){
            i=1;
        }
        for (; i<rawValue.size(); i++) {
            if (rawValue[i] >= 0x30 && rawValue[i] <= 0x39) {
                value *= 10;
                value += (rawValue[i] - 0x30);
            }
            else {
                // invalid character provided
                keyValue.type = AKVSConfig::value_type::EMPTY;
                return keyValue;
            }
            
            if (value > maxAllowed || value < 0) {
                // oops, value is too large!
                keyValue.type = AKVSConfig::value_type::EMPTY;
                return keyValue;
            }
        }
        if (isNegative) {
            value *= -1;
        }
        
        keyValue.value = new char[bytes];
        for(int i=0; i<bytes; i++) {
            keyValue.value[i] = '\0';
            keyValue.value[i] |= (uint8_t)((value & mask) >> offset);
            mask >>= 8;
            offset -= 8;
        }
        
        return keyValue;
    }
    
    AKVSStructures::key_value packagePairForSTR(AKVSStructures::key_value keyValue, std::string rawValue, int strLen) {
        
        if(rawValue.length() > strLen) {
            keyValue.setNonexistantKVPair();
        }
        else {
            keyValue.value = new char[strLen];
            for(int i=0; i<strLen; i++) {
                if(rawValue.length() > i){
                    keyValue.value[i] = rawValue[i];
                }
                else {
                    keyValue.value[i] = '\0';
                }
            }
        }
        return keyValue;
    }
    
    AKVSStructures::key_value packagePairForFloat(AKVSStructures::key_value keyValue, std::string rawValue) {
        
        int len = getValueLengthForType(AKVSConfig::value_type::FLOAT);
        if(rawValue.length() > len) {
            keyValue.setNonexistantKVPair();
        }
        else {
            
            // convert string to floating point
            const char* rawValueCstr = rawValue.c_str();
            char* end;
            float f = strtof(rawValueCstr, &end);
            if (end == rawValueCstr) {
                // conversion failed!
                keyValue.setNonexistantKVPair();
                return keyValue;
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(9) << f;
            std::string fStr = ss.str();
            
            // store value
            keyValue.value = new char[len];
            for(int i=0; i<len; i++) {
                if(fStr.length() > i){
                    keyValue.value[i] = fStr[i];
                }
                else {
                    keyValue.value[i] = '\0';
                }
            }
        }
        return keyValue;
    }
    
    AKVSStructures::key_value packagePairForDouble(AKVSStructures::key_value keyValue, std::string rawValue) {
        
        int len = getValueLengthForType(AKVSConfig::value_type::DOUBLE);
        if(rawValue.length() > len) {
            keyValue.setNonexistantKVPair();
        }
        else {
            
            // convert string to floating point
            const char* rawValueCstr = rawValue.c_str();
            char* end;
            double d = strtod(rawValueCstr, &end);
            if (end == rawValueCstr) {
                // conversion failed!
                keyValue.setNonexistantKVPair();
                return keyValue;
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(15) << d;
            std::string dStr = ss.str();
            
            // store value
            keyValue.value = new char[len];
            for(int i=0; i<len; i++) {
                if(dStr.length() > i){
                    keyValue.value[i] = dStr[i];
                }
                else {
                    keyValue.value[i] = '\0';
                }
            }
        }
        return keyValue;
    }
    
    AKVSStructures::key_value packagePairForBoolean(AKVSStructures::key_value keyValue, std::string rawValue) {
        
        bool isFalse = true;
        if (rawValue == "true" || rawValue == "t") {
            isFalse = false;
        }
        
        keyValue.value = new char[1];
        keyValue.value[0] = isFalse ? '\0' : 255;
        
        return keyValue;
    }
    
    AKVSStructures::key_value packagePair(AKVSStructures::key_value keyValue, std::string rawValue) {
        
        switch (keyValue.type) {
            case AKVSConfig::value_type::UINT8:
                return packagePairForUINT(keyValue, rawValue, 1);
            case AKVSConfig::value_type::UINT16:
                return packagePairForUINT(keyValue, rawValue, 2);
            case AKVSConfig::value_type::UINT32:
                return packagePairForUINT(keyValue, rawValue, 4);
            case AKVSConfig::value_type::UINT64:
                return packagePairForUINT(keyValue, rawValue, 8);

            case AKVSConfig::value_type::INT8:
                return packagePairForINT(keyValue, rawValue, 1);
            case AKVSConfig::value_type::INT16:
                return packagePairForINT(keyValue, rawValue, 2);
            case AKVSConfig::value_type::INT32:
                return packagePairForINT(keyValue, rawValue, 4);
            case AKVSConfig::value_type::INT64:
                return packagePairForINT(keyValue, rawValue, 8);

            case AKVSConfig::value_type::FLOAT:
                return packagePairForFloat(keyValue, rawValue);
            case AKVSConfig::value_type::DOUBLE:
                return packagePairForDouble(keyValue, rawValue);

            case AKVSConfig::value_type::BOOLEAN:
                return packagePairForBoolean(keyValue, rawValue);
                
            case AKVSConfig::value_type::CHAR16:
                return packagePairForSTR(keyValue, rawValue, 16);
            case AKVSConfig::value_type::CHAR64:
                return packagePairForSTR(keyValue, rawValue, 64);
            case AKVSConfig::value_type::CHAR256:
                return packagePairForSTR(keyValue, rawValue, 256);
            case AKVSConfig::value_type::CHAR1024:
                return packagePairForSTR(keyValue, rawValue, 1024);
                
            default:
                keyValue.type = AKVSConfig::value_type::EMPTY;
                return keyValue;
        }
    }
}

#endif /* input_shell_h */
