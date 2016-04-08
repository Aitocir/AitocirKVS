//
//  akvs.h
//  AthloiRelived
//
//  Created by Aitocir on 3/27/16.
//  Copyright © 2016 AitocirLabs. All rights reserved.
//

#ifndef akvs_h
#define akvs_h

#include <stdlib.h>
#include <string>

// Database functions
bool openDatabase(std::string filePath);
bool closeDatabase();

// Setters
bool setInt(std::string key, int value);
bool setBool(std::string key, bool value);
bool setString16(std::string key, std::string value);
bool setString64(std::string key, std::string value);
bool setString256(std::string key, std::string value);
bool setString1024(std::string key, std::string value);

// Getters
int getInt(std::string key);
bool getBool(std::string key);
std::string getString16(std::string key);
std::string getString64(std::string key);
std::string getString256(std::string key);
std::string getString1024(std::string key);

// Delete
bool deleteKey(std::string key);

#endif /* akvs_h */
