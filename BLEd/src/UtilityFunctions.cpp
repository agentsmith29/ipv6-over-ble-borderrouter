//
// Created by developer on 15.07.19.
//
#include <limits.h>
#include <sstream>

#include "UtilityFunctions.h"
#include "Logger.h"


using std::string;
namespace BLEd {
//---------------------------------------------------------------------------------------------------------------------
    int stringToInt(string input, int &num_out) {

        const char *nptr = input.c_str();                     /* string to read               */
        char *endptr = NULL;
        char buf_for_return[1024];

        num_out = std::strtol(input.c_str(), &endptr, 10);

        if (nptr == endptr) {
            Logger::write({"Number : ", input, " invalid  (no digits found, 0 returned)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -1;
        } else if (errno == ERANGE && num_out == LONG_MIN) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -2;
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -3;
        } else if (errno != 0 && num_out == 0) {
            Logger::write({"Number : ", input, " invalid  (unspecified error occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -4;
        } else if (errno == 0 && nptr && *endptr != 0) {
            Logger::write({"Number : ", input, " valid  (but additional characters remain)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -5;
        } else if (errno == 0 && nptr && !*endptr) {
            return 0;
        }

        return -6;

    }

    int stringToInt(string input) {

        const char *nptr = input.c_str();                     /* string to read               */
        char *endptr = NULL;
        char buf_for_return[1024];

        int num_out = std::strtol(input.c_str(), &endptr, 10);

        if (nptr == endptr) {
            throw ("Conversion Error : %s invalid  (no digits found, 0 returned)", input);
        } else if (errno == ERANGE && num_out == LONG_MIN) {
            throw ("Conversion Error : %s invalid  (underflow occurred)", input);
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            throw ("Conversion Error %s:  invalid  (underflow occurred)");
        } else if (errno != 0 && num_out == 0) {
            throw ("Conversion Error: %s invalid  (unspecified error occurred)", input);
        } else if (errno == 0 && nptr && *endptr != 0) {
            throw ("Conversion Error : %s valid  (but additional characters remain)", input);
        } else if (errno == 0 && nptr && !*endptr) {
            return num_out;
        }
    }


    //---------------------------------------------------------------------------------------------------------------------
    int stringToFloat(string input, float &num_out) {

        const char *nptr = input.c_str();                     /* string to read               */
        char *endptr = NULL;
        char buf_for_return[1024];

        num_out = std::strtof(input.c_str(), &endptr);

        if (nptr == endptr) {
            Logger::write({"Number : ", input, " invalid  (no digits found, 0 returned)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -1;
        } else if (errno == ERANGE && num_out == LONG_MIN) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -2;
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -3;
        } else if (errno != 0 && num_out == 0) {
            Logger::write({"Number : ", input, " invalid  (unspecified error occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -4;
        } else if (errno == 0 && nptr && *endptr != 0) {
            Logger::write({"Number : ", input, " valid  (but additional characters remain)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -5;
        } else if (errno == 0 && nptr && !*endptr) {
            return 0;
        }

        return -6;

    }

    //---------------------------------------------------------------------------------------------------------------------
    int hexToInt(string input, int &num_out) {

        const char *nptr = input.c_str();                     /* string to read               */
        char *endptr = NULL;
        char buf_for_return[1024];

        num_out = std::strtol(input.c_str(), &endptr, 16);

        if (nptr == endptr) {
            Logger::write({"Number : ", input, " invalid  (no digits found, 0 returned)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -1;
        } else if (errno == ERANGE && num_out == LONG_MIN) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -2;
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            Logger::write({"Number : ", input, " invalid  (overflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -3;
        } else if (errno != 0 && num_out == 0) {
            Logger::write({"Number : ", input, " invalid  (unspecified error occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -4;
        } else if (errno == 0 && nptr && *endptr != 0) {
            Logger::write({"Number : ", input, " valid  (but additional characters remain)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -5;
        } else if (errno == 0 && nptr && !*endptr) {
            return 0;
        }

        return -6;

    }



/*
    int stringToIntn(string input) {

        const char *nptr = input.c_str();                     // string to read               //
        char *endptr = NULL;
        char buf_for_return[1024];
        int conversion;

        conversion  = std::strtol(input.c_str(), &endptr, 10);

        if (nptr == endptr) {
            throw std::domain_error("Number : ", input, " invalid  (no digits found, 0 returned)");
        } else if (errno == ERANGE && num_out == LONG_MIN) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -2;
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            Logger::write({"Number : ", input, " invalid  (underflow occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -3;
        } else if (errno != 0 && num_out == 0) {
            Logger::write({"Number : ", input, " invalid  (unspecified error occurred)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -4;
        } else if (errno == 0 && nptr && *endptr != 0) {
            Logger::write({"Number : ", input, " valid  (but additional characters remain)"},
                          ERROR_MSG, BLE_LOG_FILE);
            return -5;
        } else if (errno == 0 && nptr && !*endptr) {
            return conversion;
        }

        return -6;

    }

*/

    //---------------------------------------------------------------------------------------------------------------------
    int HCIToInt(string input) {
        char *endptr = NULL;
        const char *nptr = input.c_str();
        std::stringstream msg;
        char buf[3];

        if(input.size() == 0) {
            msg << "Conversion Error: " << input << "No input given." << std::endl;
            throw msg.str();
        }

        if(input.size() >= 4) {
            memcpy(buf, input.c_str(), 3);
            if (strcmp(buf, "hci") ) {
                msg << "Error: " << input
                    << ", no valid descriptor found (Input as hci<n>)." << std::endl;
                throw msg.str();
            }
            input.erase(input.begin(), input.begin() + 3);
        }


        int num_out = std::strtol(input.c_str(), &endptr, 10);

        //throw  msg.str();

        if (nptr == endptr) {
            msg << "Error: " << input << " invalid, no digits found. Input as hci<n>"
            << std::endl;
            throw  msg;
        } else if (errno == ERANGE && num_out < 0) {
            msg << "Error: "<< input << " invalid, underflow occurred."
                << "Minimal Value is 0." << std::endl;
            throw  msg.str();
        } else if (errno == ERANGE && num_out == LONG_MAX) {
            msg << "Error: "<< input << " invalid. Overflow occured."
                    << "Maximum Value is " << LONG_MAX << std::endl;
            throw  msg.str();
        } else if (errno != 0 && num_out == 0) {
            msg << "Error: "<< input << " invalid. Unspecific Error. Input as hci<n> "
            << std::endl;
            throw  msg.str();
        } else if (errno == 0 && nptr && *endptr != 0) {
            msg << "Error: "<< endptr << " invalid. No description found. Input as hci<n>"
            << std::endl;
            throw  msg.str();
        } else if (errno == 0 && nptr && !*endptr) {
            return num_out;
        }
        else {
            msg << "Error: "<< endptr << " invalid. Other error. Input as hci<n>"
                << std::endl;
            throw  msg.str();
        }

    }


}