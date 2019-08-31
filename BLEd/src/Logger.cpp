//
// Created by developer on 05.06.19.
//

#include "Logger.h"
#include <chrono>
#include <mutex>
#include "Defines.h"

static ofstream ble_outfile;
static ofstream hci_outfile;

std::mutex logger_lock_;

int Logger::write(initializer_list<string> args, int log_level, int file) {
    std::unique_lock<std::mutex> lock(logger_lock_);
    ofstream *outfile;

    switch (file) {
        case BLE_LOG_FILE:
            outfile = &ble_outfile;
            break;
        case HCI_LOG_FILE:
            outfile = &hci_outfile;
            break;
        default:
            return -1;
    }

    std::string log = "";
    for (auto str : args)
        log += str;

    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char *time_s = ctime(&timenow);
    if (time_s[strlen(time_s) - 1] == '\n') time_s[strlen(time_s) - 1] = '\0';


    //<< getpid()
    writeOut(outfile, log_level, log);

}

int Logger::write(string log, int log_level, int file){
    std::unique_lock<std::mutex> lock(logger_lock_);
    ofstream *outfile;
    switch (file) {
        case BLE_LOG_FILE:
            outfile = &ble_outfile;
            break;
        case HCI_LOG_FILE:
            outfile = &hci_outfile;
            break;
        default:
            return -1;
    }
    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char *time_s = ctime(&timenow);



    if (time_s[strlen(time_s) - 1] == '\n') time_s[strlen(time_s) - 1] = '\0';

    writeOut(outfile, log_level, log);
}


int Logger::writeOut(ofstream *outfile, int log_level, string log){
    switch (log_level) {
        case FATAL_MSG:
            *outfile << RED    << "[FATAL]  " << dateNow() << ": " << log << RESET << std::endl;
            break;
        case ERROR_MSG:
            *outfile << RED    << "[ERROR]  " << dateNow() << ": " << log << RESET << std::endl;
            break;
        case WARNING_MSG:
            *outfile << YELLOW << "[WARN]   " << dateNow() << ": " << log << RESET<< std::endl;
            break;
        case SYSTEM_MSG:
            *outfile           << "[SYSTEM] " << dateNow() << ": " << log << std::endl;
            break;
        case INFO_MSG:
            *outfile           << "[INFO]   " << dateNow() << ": " << log << std::endl;
            break;
        case DEBUG_MSG:
            *outfile           << "[DEBUG]  " << dateNow() << ": " << log << std::endl;
            break;
    }
    return 0;
}

int Logger::initLogger(){
    // Create a file in /var/log
    ble_outfile.open(BLE_LOG_FILE_PATH);
    hci_outfile.open(HCI_LOG_FILE_PATH);
    write("Log created", INFO_MSG, BLE_LOG_FILE);
    write("Log created", INFO_MSG, HCI_LOG_FILE);
    return 0;
}

int Logger::closeLogger() {
    ble_outfile.close();
    hci_outfile.close();
    return 0;
}

string Logger::dateNow()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [18];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,18,"%d/%m/%y %T\0",timeinfo);
    //puts (buffer);

    return buffer;
}
