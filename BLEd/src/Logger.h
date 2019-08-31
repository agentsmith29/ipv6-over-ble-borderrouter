//
// Created by developer on 05.06.19.
//

#ifndef BLED_LOGGER_H
#define BLED_LOGGER_H

#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>


#define FATAL_MSG 1
#define ERROR_MSG 2
#define WARNING_MSG 3
#define SYSTEM_MSG 4
#define INFO_MSG 5
#define DEBUG_MSG   6

#define BLE_LOG_FILE_PATH "/tmp/6lowpan_helper.log"
#define HCI_LOG_FILE_PATH "/tmp/6low_tmp.log"

#define BLE_LOG_FILE 0
#define HCI_LOG_FILE 1

#define RED "\u001b[31m"
#define RESET "\u001b[0m"

using std::initializer_list;
using std::string;
using std::ofstream;
class Logger {

public:
    Logger();
    static int initLogger();
    static int closeLogger();
    static int write(initializer_list<string> args, int log_level, int file);
    static int write(string log, int log_level, int file);
    //static int write(string log, int log_level);

private:
    static string dateNow();
    static int writeOut(ofstream *outfile, int log_level, string log);

};


#endif //BLED_LOGGER_H
