//
// Created by developer on 15.07.19.
//

#ifndef BLED_UTILITYFUNCTIONS_H
#define BLED_UTILITYFUNCTIONS_H

#include <string>

namespace BLEd{
    int stringToInt(std::string input, int &num_out);

    int stringToInt(std::string input);

    int stringToFloat(std::string input, float &num_out);
    int hexToInt(std::string input, int &num_out);

    int HCIToInt(std::string input);
}



#endif //BLED_UTILITYFUNCTIONS_H
