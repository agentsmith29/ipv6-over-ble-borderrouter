#include <form.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>


struct data{
    uint8_t lower;
    uint8_t higher;
    uint16_t full;
};
data convert_Connection_Handle(int connection_handle);
data convert_Conn_Interval(float conn_interval);
data convert_Supervision_Timeout(float supervision_timeout);
data convert_Conn_Latency(float conn_latency);
data convert_CE_Length(float cd_length);

/**
 * float time = 4000;
    char buf[64]; std::stringstream desc;
    sprintf(buf, "0x%2.2x 0x%2.2x",  convert_Conn_Interval(time).higher, convert_Conn_Interval(time).lower);
    desc << "Connection Interval Min" << time << " ms -> " << buf;
    mvaddstr(6,5, desc.str().c_str());
    desc.clear();
 */

int main() {
    std::string input = "";

    std::cout << "Handle: ";
    std::getline (std::cin,input);
    int handle = std::atoi(input.c_str());

    std::cout << "Connection Interval Min: ";
    std::getline (std::cin,input);
    float time_connection_interval_min = std::atof(input.c_str());


    std::cout << "Connection Interval Max: ";
    std::getline (std::cin,input);
    float time_connection_interval_max = std::atof(input.c_str());



    std::cout << "Conn Latency: ";
    std::getline (std::cin,input);
    float time_conn_latency = std::atof(input.c_str());



    std::cout << "Supervision Timeout: ";
    std::getline (std::cin,input);
    float time_supervision_timeout = std::atof(input.c_str());



    std::cout << "Minimum CE Length: ";
    std::getline (std::cin,input);
    float time_minimum_ce_length = std::atof(input.c_str());


    std::cout << "Maximum CE Length: ";
    std::getline (std::cin,input);
    float time_maximum_ce_length = std::atof(input.c_str());


    char buf[128]; std::stringstream desc;
    sprintf(buf, "0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x   0x%2.2x 0x%2.2x",
            convert_Connection_Handle(handle).higher, convert_Connection_Handle(handle).lower,
            convert_Conn_Interval(time_connection_interval_min).higher,  convert_Conn_Interval(time_connection_interval_min).lower,
            convert_Conn_Interval(time_connection_interval_max).higher, convert_Conn_Interval(time_connection_interval_max).lower,
            convert_Conn_Latency(time_conn_latency).higher,convert_Conn_Latency(time_conn_latency).lower,
            convert_Supervision_Timeout(time_supervision_timeout).higher,convert_Supervision_Timeout(time_supervision_timeout).lower,
            convert_CE_Length(time_minimum_ce_length).higher, convert_CE_Length(time_minimum_ce_length).lower,
            convert_CE_Length(time_maximum_ce_length).higher, convert_CE_Length(time_maximum_ce_length).lower);

    std::cout << "sudo hcitool cmd 0x08 0x13  " << buf << std::endl;


    sprintf(buf, "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x",
            convert_Connection_Handle(handle).higher, convert_Connection_Handle(handle).lower,
            convert_Conn_Interval(time_connection_interval_min).higher,  convert_Conn_Interval(time_connection_interval_min).lower,
            convert_Conn_Interval(time_connection_interval_max).higher, convert_Conn_Interval(time_connection_interval_max).lower,
            convert_Conn_Latency(time_conn_latency).higher,convert_Conn_Latency(time_conn_latency).lower,
            convert_Supervision_Timeout(time_supervision_timeout).higher,convert_Supervision_Timeout(time_supervision_timeout).lower,
            convert_CE_Length(time_minimum_ce_length).higher, convert_CE_Length(time_minimum_ce_length).lower,
            convert_CE_Length(time_maximum_ce_length).higher, convert_CE_Length(time_maximum_ce_length).lower);

    std::cout << "sudo hcitool cmd 08 13 " << buf << std::endl;
}

data convert_Connection_Handle(int connection_handle)
{

    uint16_t res_hex = static_cast<uint16_t>(connection_handle);

    data res_hex_split;
    res_hex_split.higher=res_hex & 0xff;
    res_hex_split.lower=res_hex >> 8;
    res_hex_split.full = res_hex;

    return res_hex_split;
}

///
/// \param conn_interval in ms
/// \return

data convert_Conn_Interval(float conn_interval)
{
    float multiplicator = 1.25;
    multiplicator = 1;
    float time_min = 7.5, time_max = 4000; // in ms

    double result = conn_interval / multiplicator;
    uint16_t res_hex = static_cast<uint16_t>(result);

    data res_hex_split;
    res_hex_split.higher=res_hex & 0xff;
    res_hex_split.lower=res_hex >> 8;
    res_hex_split.full = res_hex;

    return res_hex_split;
}




data convert_Conn_Latency(float conn_latency)
{
    //float multiplicator = 1;
    // float time_min = 7.5, time_max = 4000; // in ms

    double result = conn_latency;
    uint16_t res_hex = static_cast<uint16_t>(result);

    data res_hex_split;
    res_hex_split.higher=res_hex & 0xff;
    res_hex_split.lower=res_hex >> 8;
    res_hex_split.full = res_hex;

    return res_hex_split;
}


data convert_Supervision_Timeout(float supervision_timeout)
{
    float multiplicator = 10;
    multiplicator = 1;
    float time_min = 100, time_max = 32000; // in ms

    double result = supervision_timeout / multiplicator;
    uint16_t res_hex = static_cast<uint16_t>(result);

    data res_hex_split;
    res_hex_split.higher=res_hex & 0xff;
    res_hex_split.lower=res_hex >> 8;
    res_hex_split.full = res_hex;

    return res_hex_split;
}



data convert_CE_Length(float cd_length)
{
    float multiplicator = 0.625;
    multiplicator = 1;
    //float time_min = 100, time_max = 32000; // in ms

    double result = cd_length / multiplicator;
    uint16_t res_hex = static_cast<uint16_t>(result);

    data res_hex_split;
    res_hex_split.higher=res_hex & 0xff;
    res_hex_split.lower=res_hex >> 8;
    res_hex_split.full = res_hex;


    return res_hex_split;
}