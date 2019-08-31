//
// Created by developer on 05.06.19.
//

#include "HwAddress.h"

HwAddress::HwAddress(string address, int address_type)
{
    hw_address_ = address;
    hw_address_type_ = address_type;
}

HwAddress::HwAddress()
{
    hw_address_ = "unknown";
    hw_address_type_ = DUMMY_ADDRESS;
}

string HwAddress::getAddress(){
    return hw_address_;
}

string HwAddress::getHexAddress(){
        return hw_address_;
}

int HwAddress::getAddressType(){
    return hw_address_type_;
}

int HwAddress::setAddress(string address, int type){
    hw_address_ = address;
    hw_address_type_ = type;
}

int HwAddress::setAddress(bdaddr_t address, int type){
    char buf[18];
    sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &address.b[5], &address.b[4], &address.b[3],
               &address.b[2], &address.b[1], &address.b[0]);
    buf[17] = '\0';

    hw_address_ = buf;
    hw_address_type_ = type;
}


bool addrComp(HwAddress address_1, HwAddress address_2){

    if(!strcmp(address_1.getAddress().c_str(), address_2.getAddress().c_str())
        && (address_1.getAddressType() == address_2.getAddressType())) {
        return true;
    }
    return false;

}
