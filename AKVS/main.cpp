//
//  main.cpp
//  AKVS
//
//  Created by Aitocir on 10/23/15.
//  Copyright © 2015 AitocirLabs. All rights reserved.
//

#include <algorithm>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <vector>

#include "config.h"
#include "input_shell.h"
#include "structures.h"

// addresses (un)signed char platform differences
#ifdef __CHAR_UNSIGNED__
#define CHAR_FACTOR 0
#else
#define CHAR_FACTOR 256
#endif

using namespace std;
using namespace AKVSConfig;
using namespace AKVSInputShell;
using namespace AKVSStructures;


uint16_t getHashForKeyAndType(key_value entry) {
    
    // DEBUG: put everything in one bucket
    // testing removal/reuse code
    //return 0;
    
    uint32_t p = 16777619;
    uint32_t o = 2166136261;
    uint16_t m = 0x0FFF;
    
    uint32_t h32 = o;
    for(int i=0; i<CONFIG_keySize; i++){
        h32 = h32 ^ entry.key[i];
        h32 = h32 * p;
    }
    
    // mask down to 16 bits
    uint16_t h16 = h32 & 0x0000FFFF;
    
    // clear top 4 bits for value-type modding
    h16 &= m;
    
    // get 4 bits for value-type and original hash
    uint16_t len = getValueLengthForType(entry.type);
    set<int>::iterator it(CONFIG_weights[len].begin());
    advance(it, h32 % CONFIG_weights[len].size());
    uint16_t typeMod = *it; // guaranteed to be in 0x000F nybble
    typeMod <<= 12;
    
    // add 4 bits to the final hash
    h16 += typeMod;
    return h16;
}

//
// findKey is used to check if the database contains a key
//      if it does: returns the byte index of the existing key
//      if it doesn't: returns first available byte index key COULD be added and the next ptr that would need updated
//
key_idx searchForKeyInBucket(key_value request, uint16_t hash) {
    
    // declare idx struct
    key_idx result;
    uint32_t hashHead = (((uint32_t)hash)*4)+CONFIG_HASHTABLE_START;
    uint32_t idx;
    
    char* hashPtr = new char[4];
    db_file.seekg(hashHead);
    db_file.read(hashPtr, 4);
    idx = hashPtr[0] & 0x80 ? CHAR_FACTOR+hashPtr[0] : hashPtr[0];
    idx <<= 8;
    idx += hashPtr[1] & 0x80 ? CHAR_FACTOR+hashPtr[1] : hashPtr[1];
    idx <<= 8;
    idx += hashPtr[2] & 0x80 ? CHAR_FACTOR+hashPtr[2] : hashPtr[2];
    idx <<= 8;
    idx += hashPtr[3] & 0x80 ? CHAR_FACTOR+hashPtr[3] : hashPtr[3];
    
    // check to see if hash bucket is empty
    if (idx == 0) {
        result.exists = false;
        result.index = 0; // 0 == EOF
        result.tail = (hash*4) + CONFIG_HASHTABLE_START;
        return result;
    }
    // if it isn't traverse looking for key
    else {
        uint32_t tailIdx = (hash*4) + CONFIG_HASHTABLE_START;
        uint32_t firstEmptyIdx = 0;
        uint32_t firstEmptyTail = 0;

        while (idx >= (CONFIG_HASHTABLE_START + (65536*4))) {
            // check to see if this is our key
            bool isKey = true;
            bool isErased = true;
            char* keyFromFile = new char[CONFIG_keySize];
            db_file.seekg(idx);
            db_file.read(keyFromFile, CONFIG_keySize);
            for(int i=0; i<CONFIG_keySize; i++) {
                if (request.key[i] != keyFromFile[i]) {
                    // not a match
                    isKey = false;
                }
                if (keyFromFile[i] != '\0') {
                    isErased = false;
                }
            }
            
            if (!isKey) {
                
                if (isErased && firstEmptyIdx == 0) {
                    firstEmptyIdx = idx;
                    firstEmptyTail = tailIdx;
                }
                
                tailIdx = idx+(CONFIG_keySize+1+getValueLengthForType(request.type));
                db_file.seekg(tailIdx);
                db_file.read(hashPtr, 4);
                idx = hashPtr[0] & 0x80 ? CHAR_FACTOR+hashPtr[0] : hashPtr[0];
                idx <<= 8;
                idx += hashPtr[1] & 0x80 ? CHAR_FACTOR+hashPtr[1] : hashPtr[1];
                idx <<= 8;
                idx += hashPtr[2] & 0x80 ? CHAR_FACTOR+hashPtr[2] : hashPtr[2];
                idx <<= 8;
                idx += hashPtr[3] & 0x80 ? CHAR_FACTOR+hashPtr[3] : hashPtr[3];
                cout << "idx: " << idx << endl;
            }
            else {
                result.exists = true;
                result.index = idx;
                result.tail = tailIdx;
                return result;
            }
        }
        
        // if we get here, and idx isn't 0,
        // something went terribly wrong!
        // (hash list ptr went back to reserved first indexes)
        result.exists = false;
        result.tail = firstEmptyTail ? firstEmptyTail : tailIdx;
        result.index = firstEmptyIdx ? firstEmptyIdx : idx; // 0 will be treated as EOF
        return result;
    }

}


key_idx findKey(key_value request) {
    
    // hash key
    uint16_t hash = getHashForKeyAndType(request);
    return searchForKeyInBucket(request, hash);
}

key_idx findKeyWithoutType(key_value request) {
    
    // hash key against all possible types
    // only save unique hashes
    // (collisions occur on values with the same length)
    //
    set<uint16_t> hashes;
    key_idx keyIndex;
    vector<value_type> types = getValueTypes();
    for(int i=0; i<types.size(); i++) {
        request.type = types[i];
        uint16_t hash = getHashForKeyAndType(request);
        if(hashes.find(hash) == hashes.end()) {
            // haven't tried this hash bucket yet
            hashes.insert(hash);
            keyIndex = searchForKeyInBucket(request, hash);
            if (keyIndex.exists) {
                return keyIndex;
            }
        }
        
    }
    
    // if we get here, none of the hashes worked out
    //
    keyIndex.exists = false;
    keyIndex.index = 0;
    keyIndex.tail = 0;
    return keyIndex;
}

key_value getValueForKey(key_value key) {
    
    key_idx keyLookup = findKey(key);
    
    if (!keyLookup.exists) {
        key.setNonexistantKVPair();
    }
    else {
        char* rawValueType;
        db_file.seekg(keyLookup.index + CONFIG_keySize);
        db_file.read(rawValueType, 1);
        key.type = getValueTypeForRawValue((uint8_t)*rawValueType);
        
        key.value = new char[getValueLengthForType(key.type)];
        db_file.read(key.value, getValueLengthForType(key.type));
    }
    
    return key;
}

bool removeKey(key_value key) {
    
    key_idx keyLookup = findKeyWithoutType(key);
    
    if (!keyLookup.exists) {
        // key already doesn't exist
        return false;
    }
    
    // we get here, the key exists
    // let's null it out
    db_file.seekg(keyLookup.index + CONFIG_keySize);
    char* rawValue = new char[1];
    db_file.read(rawValue, 1);
    value_type type = getValueTypeForRawValue((uint8_t)*rawValue);
    int valueLength = getValueLengthForType(type);
    char* blankRecord = new char[CONFIG_keySize+1+valueLength];
    fill(blankRecord, blankRecord+CONFIG_keySize+1+valueLength, '\0');
    
    db_file.seekg(keyLookup.index);
    db_file.write(blankRecord, CONFIG_keySize+1+valueLength);
    delete [] blankRecord;
    delete [] rawValue;

    // notice we're leaving the ptr to the next record in place
    // this is by design
    
    // TODO: maybe slurp up last bucket item to this blank spot so that
    // all blank records are at the end of hash buckets?
    return true;
}

void setKeyValuePair(key_value kvPair) {
    
    // check that the key doesn't already exist?
    key_idx keyLookup = findKey(kvPair);
    uint16_t valueLen = getValueLengthForType(kvPair.type);
    
    if (keyLookup.exists) {
        // key already exists!
        // let's do an update instead of an add
        char* entry = new char[valueLen];
        for (int i=0; i<valueLen; i++) {
            entry[i] = kvPair.value[i];
        }
        
        //
        // overwrite existing value ONLY (leave type and key)
        //
        uint32_t valueIdx = keyLookup.index + CONFIG_keySize + 1; // +1 for value type byte
        db_file.seekg(valueIdx);
        db_file.write(entry, valueLen);
        
        delete [] entry;
    }
    else {
    
        // if key doesn't exist, keyLookup contains
        // the .tail has the byte index of the pointer
        // that needs to address this new key
        // the key itself will simply be appended to the file
        
        //
        // load up record
        //
        char* entry = new char[CONFIG_keySize+1+valueLen+4];
        for(int i=0; i<CONFIG_keySize; i++){
            entry[i] = kvPair.key[i];
        }
        entry[CONFIG_keySize] = (uint8_t)kvPair.type;
        for (int i=0; i<valueLen; i++) {
            entry[i+CONFIG_keySize+1] = kvPair.value[i];
        }
        entry[CONFIG_keySize+1+valueLen] = '\0';
        entry[CONFIG_keySize+1+valueLen+1] = '\0';
        entry[CONFIG_keySize+1+valueLen+2] = '\0';
        entry[CONFIG_keySize+1+valueLen+3] = '\0';
        
        //
        // add record to database
        // use an empty record from the bucket, otherwise append it
        //
        if (keyLookup.index == 0 || keyLookup.index >= db_fileSize) {
            
            // we're appending this one!
            // write record to end of file
            db_file.seekg(0, db_file.end);
            long long newRecordIndex = db_file.tellg();
            db_file.write(entry, CONFIG_keySize+1+valueLen+4);
            
            // now, go connect the hash bucket ptr
            char* hashPtr = new char[4];
            hashPtr[0] = (uint8_t)((newRecordIndex & 0xFF000000) >> 24);
            hashPtr[1] = (uint8_t)((newRecordIndex & 0x00FF0000) >> 16);
            hashPtr[2] = (uint8_t)((newRecordIndex & 0x0000FF00) >> 8);
            hashPtr[3] = (uint8_t)((newRecordIndex & 0x000000FF));
            db_file.seekg(keyLookup.tail);
            db_file.write(hashPtr, 4);
            
            // update file size
            db_file.seekg(0, db_file.end);
            cout << "Old Filesize: " << db_fileSize;
            db_fileSize = (uint32_t)db_file.tellg();
            cout << ", new filesize: " << db_fileSize << endl;
            
            delete [] hashPtr;
        }
        else {
            
            // we're reusing a previously deleted record spot!
            db_file.seekg(keyLookup.index);
            db_file.write(entry, CONFIG_keySize+1+valueLen+4);
        }

        delete [] entry;
    }
}



cmd_result processSet(vector<string> args) {
    
    cmd_result result;
    
    // DEBUG: force 6 args (KEY x TYPE y VALUE z)
    if (args.size() != 6){
        result.code = 2;
        result.message = "ERROR: invalid number of parameters to SET command.";
        return result;
    }
    
    // DEBUG: validate nouns
    if (args[0] != "KEY" || args[2] != "TYPE" || args[4] != "VALUE") {
        result.code = 3;
        result.message = "ERROR: invalid AQL nouns phrases/ordering given to ADD command.";
        return result;
    }
    
    // DEBUG: validate key
    if (args[1].length() > CONFIG_keySize) {
        result.code = 4;
        result.message = "ERROR: provided key is too long for database.";
        return result;
    }
    
    // DEBUG: validate type
    value_type type = getValueTypeByName(args[3]);
    if(type == value_type::EMPTY) {
        result.code = 5;
        result.message = "ERROR: provided value type is invalid.";
        return result;
    }
    
    // DEBUG: validate value
    key_value newPair;
    newPair.setKey(args[1]);
    newPair.type = type;
    newPair = packagePair(newPair, args[5]);
    
    if (newPair.type == value_type::EMPTY) {
        // value is malformed
        result.code = 6;
        result.message = "ERROR: provided value is invalid for provided value type.";
        return result;
    }
    else {
        setKeyValuePair(newPair);
        result.code = 0;
        result.message = "I think I added your key-value pair...";
        result.content = newPair;
        return result;
    }
    
}

cmd_result processGet(vector<string> args) {
    cmd_result result;
    
    // DEBUG: force 4 args (KEY x TYPE y)
    if (args.size() != 4){
        result.code = 28;
        result.message = "ERROR: invalid number of parameters to GET command.";
        return result;
    }
    
    // DEBUG: validate nouns
    if (args[0] != "KEY" || args[2] != "TYPE") {
        result.code = 29;
        result.message = "ERROR: invalid AQL nouns phrases/ordering given to GET command.";
        return result;
    }
    
    // DEBUG: validate key
    if (args[1].length() > CONFIG_keySize) {
        result.code = 30;
        result.message = "ERROR: provided key is too long for database.";
        return result;
    }
    
    // DEBUG: validate type
    value_type type = getValueTypeByName(args[3]);
    if(type == value_type::EMPTY) {
        result.code = 31;
        result.message = "ERROR: provided value type is invalid.";
        return result;
    }
    
    key_value newPair;
    newPair.setKey(args[1]);
    newPair.type = type;
    newPair = getValueForKey(newPair);
    
    if (newPair.type == value_type::EMPTY) {
        // value is malformed
        result.code = 6;
        result.message = "ERROR: key missing, or key is not of provided type.";
        result.content = newPair;
        return result;
    }
    else {
        result.code = 0;
        result.message = "Retrieved value.";
        result.content = newPair;
        return result;
    }
    
    return result;
}

cmd_result processDelete(vector<string> args) {
    cmd_result result;
    
    // DEBUG: force 2 args (KEY x)
    if (args.size() != 2){
        result.code = 32;
        result.message = "ERROR: invalid number of parameters to DELETE command.";
        return result;
    }
    
    // DEBUG: validate nouns
    if (args[0] != "KEY") {
        result.code = 33;
        result.message = "ERROR: invalid AQL nouns phrases/ordering given to DELETE command.";
        return result;
    }
    
    // DEBUG: validate key
    if (args[1].length() > CONFIG_keySize) {
        result.code = 34;
        result.message = "ERROR: provided key is too long for database.";
        return result;
    }
    
    key_value newPair;
    newPair.setKey(args[1]);
    
    bool deleteSucceeded = removeKey(newPair);
    if(deleteSucceeded) {
        result.code = 0;
        result.content = newPair;
        result.message = "Deleted key successfully.";
    }
    else {
        result.code = 35;
        result.content = newPair;
        result.message = "Delete failed; could not find key.";
    }
    
    return result;
}

cmd_result processCommand(string cmd) {
    
    cmd_result result;
    
    // parse AQL text
    // example commands:
    //
    //  ADD KEY x TYPE y VALUE z
    //  GET KEY x TYPE y
    //  UPDATE KEY x TYPE y VALUE z
    //  DELETE KEY x
    //
    stringstream ss(cmd);
    string term;
    getline(ss, term, ' ');
    for(int i=0; i<term.length(); i++) {
        if (term[i] >= 'A' && term[i] <= 'Z') {
            term[i] += 0x20;
        }
    }
    vector<string> terms;
    string tmp;
    while (getline(ss, tmp, ' ')) {
        terms.push_back(tmp);
    }
    
    if (term == "set") {
        return processSet(terms);
    } else if (term == "get") {
        return processGet(terms);
    } else if (term == "delete") {
        return processDelete(terms);
    }
    
    // hmmm, no valid command word
    result.code = 1;
    result.message = "ERROR: No valid leading AQL command verb.";
    return result;
}

bool writeNewAKVSdb(const char* fileName) {
    
    // 0) fail if file already exists
    if (ifstream(fileName)) {
        return false;
    }
    
    // 1) write the "AKVS" header
    char* fileBytes = new char[CONFIG_HASHTABLE_START + (65536*4)];
    fileBytes[0] = 'A';
    fileBytes[1] = 'K';
    fileBytes[2] = 'V';
    fileBytes[3] = 'S';
    
    // 2) write the key size
    fileBytes[4] = '\x00';
    fileBytes[5] = '\x20';
    
    // 2) write the bytes for bucket weights
    // TODO: accept params to adjust this away from default
    // default is one for each size
    fileBytes[6] = '\0';
    fileBytes[7] = '\x01';
    fileBytes[8] = '\x02';
    fileBytes[9] = '\x03';
    fileBytes[10] = '\x05';
    fileBytes[11] = '\x08';
    fileBytes[12] = '\x0C';
    fileBytes[13] = '\x10';
    
    // 3) write the null bytes where hash bucket head ptrs will go
    fill(fileBytes+14, fileBytes+14+(65536*4), '\0');
    
    // actually commit to disk
    try {
        db_file = fstream(fileName, fstream::binary);
        db_file.open(fileName, ios_base::in | ios_base::out | ios_base::trunc | ios_base::binary);
        db_file.write(fileBytes, CONFIG_HASHTABLE_START + (65536*4));
        db_file.close();
    }
    catch (...) {
        cout << "hmmm....";
        return false;
    }
    
    return true;
}

int main(int argc, const char * argv[]) {
    
    // create a new file if this filepath does not exist
    // try to open the database if it does exist
    if (!ifstream(argv[1])) {
        // create a new default empty database
        if (!writeNewAKVSdb(argv[1])) {
            // writing new database failed!
            cout << "ERROR: didn't find existing database, and failed to write a new one. Check your access permissions on the file path.";
            return -1;
        }
    }
    //else {
        // open AKVS file
        db_file = fstream(argv[1], fstream::binary);
        db_file.open(argv[1]);
    //}

    // verify the database header
    char* buffer = new char[CONFIG_HASHTABLE_START];
    db_file.seekg(0);

    db_file.read(buffer, CONFIG_HASHTABLE_START);
    if (buffer[0] != 'A' || buffer[1] != 'K' || buffer[2] != 'V' || buffer[3] != 'S'){
        // wrong file leading 4 bytes
        cout << "ERROR: Database seems to be malformed! (Missing database header 'AKVS'\n)";
        return 2;
    }
    
    // get the key length for this database
    uint16_t keyLength = (buffer[4] << 8) + buffer[5];
    if (keyLength == 0) {
        // error: key length cannot be zero
        cout << "ERROR: Database seems to be malformed! (Key length is not allowed to be zero (0)\n)";
        return 3;
    }
    CONFIG_keySize = keyLength;
    
    // get value length weightings for this database
    for (int i=0; i<8; i++) {
        uint32_t weight = 1 << buffer[6+i];

        if(CONFIG_weights.find(weight) == CONFIG_weights.end()){
            // not a valid weight
            cout << "ERROR: Database seems to be malformed! (Invalid value-length weight)";
            return 4;
        }
        else {
            CONFIG_weights[weight].insert(i+8);
        }
    }
    delete [] buffer;
    
    // database opened successfully!
    // inform the user and let the live session begin!
    cout << "Opened AKVS database!\n\tKey length: " << CONFIG_keySize \
    << "\n\tToo lazy to print key weights right meow...\n\n";
    
    // get database size
    db_file.seekg(0, db_file.end);
    db_fileSize = (uint32_t)db_file.tellg();
    
    if (argc >= 3) {
        //
        // extra params means command passed for non-interactive mode
        //
        cmd_result result = processCommand(argv[2]);
        cout << result.message;
        cout << endl;
        if (result.code == 0) {
            cout << result.contentString();
            cout << endl;
        }
        return 0;
    }
    else {
        //
        // just the database file provided
        // we will run in interactive mode
        //
        bool userHasQuit = false;
        cmd_result commandResult;
        commandResult.message = "";
        commandResult.code = -1;
        do {
            
            // 0) print out errors/feedback
            cout << commandResult.message;
            cout << endl;
            
            if (commandResult.code == 0){
                cout << commandResult.contentString();
                cout << endl;
            }
            
            // 1) print out prompt
            cout << "\n> ";
            
            // 2) gather user input
            string userInput;
            getline(cin, userInput);
            cin.clear();
            
            // 3) process input, set errors, etc
            commandResult = processCommand(userInput);
            
        } while (!userHasQuit);
        
        return commandResult.code;
    }
    
}
