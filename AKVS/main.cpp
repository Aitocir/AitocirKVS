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

#include "akvs.h"

using namespace std;


int main(int argc, const char * argv[]) {
    
    string dbPath = string(argv[1]);
    if (!openDatabase(dbPath)) {
        cout << "";
        return 1;
    }
    
    if (argc >= 3) {
        //
        // extra params means command passed for non-interactive mode
        //
        // TODO: pass argv through API for typed commands
        return 0;
    }
    else {
        //
        // just the database file provided
        // we will run in interactive mode
        //
        // TODO: run interactive loop for repeated commands
//        bool userHasQuit = false;
//        cmd_result commandResult;
//        commandResult.message = "";
//        commandResult.code = -1;
//        do {
//
//            // 0) print out errors/feedback
//            cout << commandResult.message;
//            cout << endl;
//
//            if (commandResult.code == 0){
//                cout << commandResult.contentString();
//                cout << endl;
//            }
//
//            // 1) print out prompt
//            cout << "\n> ";
//
//            // 2) gather user input
//            string userInput;
//            getline(cin, userInput);
//            cin.clear();
//
//            // 3) process input, set errors, etc
//            commandResult = processCommand(userInput);
//            
//        } while (!userHasQuit);
//        
//        return commandResult.code;
        return 0;
    }
    
}
