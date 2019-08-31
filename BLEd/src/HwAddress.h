//
// Created by developer on 05.06.19.
//

#ifndef BLED_HWADDRESS_H
#define BLED_HWADDRESS_H
#include <string>
#include "bluetooth.h"

using std::string;

static constexpr int NONE_ADDRESS = -2;
static constexpr int DUMMY_ADDRESS = -1;
static constexpr int PUBLIC_ADDRESS = 1;
static constexpr int RANDOM_STATIC_ADDRESS = 2;
static constexpr int PRIVATE_RESOLVABLE_ADDRESS = 3;
static constexpr int PRIVATE_NON_RESOLVABLE_ADDRESS = 4;

class HwAddress {

public:
    HwAddress();
    HwAddress(string address, int type);

    string getAddress();

    int getAddressType();

    string getHexAddress();

    string generateLocalLinkAddress();

    int setAddress(string address, int type);

    int setAddress(bdaddr_t address, int type);




private:
    string hw_address_;
    bdaddr_t hw_hex_address[7];
    int hw_address_type_;

};

/// A function which compares two HwAddresses
/// \param address_1 First address to compare
/// \param address_2 Second address to compare
/// \return True if the same, false otherwise
bool addrComp(HwAddress address_1, HwAddress address_2);



#endif //BLED_HWADDRESS_H
